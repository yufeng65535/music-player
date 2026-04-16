#include "core/AudioEngine.h"

#include <QDebug>
#include <cmath>
#include "kiss_fft.h"

// Standard 10-band EQ center frequencies
static const float kEqFrequencies[10] = {
    31, 62, 125, 250, 500, 1000, 2000, 4000, 8000, 16000
};

AudioEngine::AudioEngine(QObject* parent)
    : QObject(parent)
{
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_audioOutput->setVolume(0.7f);
    m_player->setAudioOutput(m_audioOutput);

    // QAudioDecoder for visualization data
    // (QAudioProbe doesn't exist in Qt 6.2, so we decode in parallel)
    m_decoder = new QAudioDecoder(this);

    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Float);
    m_decoder->setAudioFormat(format);

    connect(m_decoder, &QAudioDecoder::bufferReady,
            this, [this]() {
        QAudioBuffer buffer = m_decoder->read();
        if (buffer.isValid()) {
            processAudioBuffer(buffer);
        }
    });
    connect(m_decoder, &QAudioDecoder::finished,
            this, [this]() { m_decodingActive = false; });

    // Connect player signals
    connect(m_player, &QMediaPlayer::playbackStateChanged,
            this, &AudioEngine::stateChanged);
    connect(m_player, &QMediaPlayer::positionChanged,
            this, &AudioEngine::positionChanged);
    connect(m_player, &QMediaPlayer::durationChanged,
            this, &AudioEngine::durationChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged,
            this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            emit playbackFinished();
        }
    });
    connect(m_player, QOverload<QMediaPlayer::Error, const QString&>::of(&QMediaPlayer::errorOccurred),
            this, [this](QMediaPlayer::Error, const QString& msg) {
        emit errorOccurred(msg);
    });

    // Init FFT (1024-point)
    m_fftInput.resize(1024);
    m_fftOutput.resize(2048);
    m_fftCfg = kiss_fft_alloc(1024, 0, nullptr, nullptr);
}

void AudioEngine::load(const QUrl& url) {
    m_player->setSource(url);

    // Set up decoder for the same source
    m_decoder->stop();
    m_decoder->setSource(url);
    m_decodingActive = false;
}

void AudioEngine::play() {
    m_player->play();

    // Start decoder if not already running
    if (m_decoder->source().isValid() && !m_decodingActive) {
        m_decodingActive = true;
        m_decoder->start();
    }
}

void AudioEngine::pause() {
    m_player->pause();
    m_decoder->stop();
    m_decodingActive = false;
}

void AudioEngine::stop() {
    m_player->stop();
    m_decoder->stop();
    m_decodingActive = false;
}

void AudioEngine::seek(int64_t positionMs) {
    m_player->setPosition(positionMs);

    // Restart decoder from new position
    m_decoder->stop();
    m_decodingActive = false;
    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        m_decodingActive = true;
        m_decoder->start();
    }
}

void AudioEngine::setVolume(float vol) {
    m_audioOutput->setVolume(qBound(0.0f, vol, 1.0f));
}

float AudioEngine::volume() const {
    return m_audioOutput->volume();
}

QMediaPlayer::PlaybackState AudioEngine::state() const {
    return m_player->playbackState();
}

int64_t AudioEngine::position() const {
    return m_player->position();
}

int64_t AudioEngine::duration() const {
    return m_player->duration();
}

void AudioEngine::setEqBand(int band, float gainDb) {
    if (band >= 0 && band < 10) {
        m_eqBands[band] = qBound(-12.0f, gainDb, 12.0f);
    }
}

float AudioEngine::eqBand(int band) const {
    if (band >= 0 && band < 10) return m_eqBands[band];
    return 0.0f;
}

bool AudioEngine::eqEnabled() const {
    return m_eqEnabled;
}

void AudioEngine::setEqEnabled(bool enabled) {
    m_eqEnabled = enabled;
}

void AudioEngine::processAudioBuffer(const QAudioBuffer& buffer) {
    const QAudioFormat format = buffer.format();
    if (!format.isValid()) return;

    int channelCount = format.channelCount();

    // Extract float samples from first channel
    const float* data = buffer.constData<float>();
    if (!data) return;

    int sampleCount = buffer.sampleCount() / channelCount;
    if (sampleCount == 0) return;

    // Take up to 1024 samples, apply Hann window
    int n = qMin(sampleCount, 1024);
    for (int i = 0; i < n; ++i) {
        float window = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (n - 1)));
        m_fftInput[i] = data[i * channelCount] * window;
    }
    for (int i = n; i < 1024; ++i) {
        m_fftInput[i] = 0;
    }

    // Execute FFT
    kiss_fft_cfg cfg = static_cast<kiss_fft_cfg>(m_fftCfg);
    kiss_fft_cpx* fin = reinterpret_cast<kiss_fft_cpx*>(m_fftInput.data());
    kiss_fft_cpx* fout = reinterpret_cast<kiss_fft_cpx*>(m_fftOutput.data());
    kiss_fft(cfg, fin, fout);

    // Compute magnitudes (first N/2 bins)
    QList<float> magnitudes;
    magnitudes.reserve(512);
    for (int i = 0; i < 512; ++i) {
        float re = m_fftOutput[i * 2];
        float im = m_fftOutput[i * 2 + 1];
        magnitudes.append(std::sqrt(re * re + im * im) / 1024.0f);
    }

    emit spectrumDataReady(magnitudes);
}
