#pragma once

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QAudioDecoder>
#include <QAudioBuffer>
#include <array>
#include <QString>

class AudioEngine : public QObject {
    Q_OBJECT
public:
    explicit AudioEngine(QObject* parent = nullptr);

    // Playback
    void load(const QUrl& url);
    void play();
    void pause();
    void stop();
    void seek(int64_t positionMs);

    // Volume: 0.0 - 1.0
    void setVolume(float volume);
    float volume() const;

    // State
    QMediaPlayer::PlaybackState state() const;
    int64_t position() const;
    int64_t duration() const;

    // EQ: 10 bands (31, 62, 125, 250, 500, 1K, 2K, 4K, 8K, 16K Hz)
    // gain: -12.0 to +12.0 dB
    void setEqBand(int band, float gainDb);
    float eqBand(int band) const;
    bool eqEnabled() const;
    void setEqEnabled(bool enabled);

signals:
    void stateChanged(QMediaPlayer::PlaybackState);
    void positionChanged(int64_t positionMs);
    void durationChanged(int64_t durationMs);
    void spectrumDataReady(const QList<float>& magnitudes);
    void playbackFinished();
    void errorOccurred(const QString& message);

private:
    void processAudioBuffer(const QAudioBuffer& buffer);

    QMediaPlayer* m_player = nullptr;
    QAudioOutput* m_audioOutput = nullptr;

    // Decoder for visualization (separate from playback)
    QAudioDecoder* m_decoder = nullptr;
    bool m_decodingActive = false;

    // EQ state
    bool m_eqEnabled = false;
    std::array<float, 10> m_eqBands{0};

    // FFT workspace
    std::vector<float> m_fftInput;
    std::vector<float> m_fftOutput;
    void* m_fftCfg = nullptr;
};
