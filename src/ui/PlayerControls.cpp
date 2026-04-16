#include "ui/PlayerControls.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QFont>
#include "core/AudioEngine.h"

PlayerControls::PlayerControls(AudioEngine* audio, QWidget* parent)
    : QWidget(parent), m_audio(audio)
{
    setupUI();
}

void PlayerControls::setupUI() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(12);

    // Play controls
    auto* btnPrev = new QPushButton("⏮", this);
    btnPrev->setFixedSize(32, 32);
    btnPrev->setToolTip("Previous");

    m_playBtn = new QPushButton("▶", this);
    m_playBtn->setFixedSize(40, 40);
    m_playBtn->setToolTip("Play/Pause");
    QFont playFont = m_playBtn->font();
    playFont.setPointSize(14);
    m_playBtn->setFont(playFont);

    auto* btnNext = new QPushButton("⏭", this);
    btnNext->setFixedSize(32, 32);
    btnNext->setToolTip("Next");

    layout->addWidget(btnPrev);
    layout->addWidget(m_playBtn);
    layout->addWidget(btnNext);

    // Progress slider
    m_progressSlider = new QSlider(Qt::Horizontal, this);
    m_progressSlider->setMinimum(0);
    m_progressSlider->setMaximum(100);
    m_progressSlider->setValue(0);
    layout->addWidget(m_progressSlider, 1);

    // Time display
    m_timeLabel = new QLabel("0:00", this);
    m_timeLabel->setFixedWidth(45);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_timeLabel);

    layout->addWidget(new QLabel("/", this));

    m_durationLabel = new QLabel("0:00", this);
    m_durationLabel->setFixedWidth(45);
    m_durationLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_durationLabel);

    // Volume
    layout->addWidget(new QLabel("🔊", this));
    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(70);
    m_volumeSlider->setFixedWidth(80);
    layout->addWidget(m_volumeSlider);

    // ── Connections ─────────────────────────────────────────

    // Play button
    connect(m_playBtn, &QPushButton::clicked, this, [this]() {
        if (m_audio->state() == QMediaPlayer::PlayingState) {
            m_audio->pause();
        } else {
            m_audio->play();
        }
    });

    connect(m_audio, &AudioEngine::stateChanged,
            this, &PlayerControls::updatePlayButton);

    // Progress
    connect(m_audio, &AudioEngine::positionChanged, this, [this](int64_t pos) {
        if (!m_userDragging && m_audio->duration() > 0) {
            m_progressSlider->setValue(
                static_cast<int>(pos * 100 / m_audio->duration()));
        }
        updateTimeDisplay();
    });

    connect(m_audio, &AudioEngine::durationChanged, this, [this](int64_t dur) {
        m_progressSlider->setMaximum(dur > 0 ? static_cast<int>(dur) : 0);
        int64_t totalSec = dur / 1000;
        m_durationLabel->setText(
            QString("%1:%2").arg(totalSec / 60).arg(totalSec % 60, 2, 10, QChar('0')));
    });

    // Seek
    connect(m_progressSlider, &QSlider::sliderPressed,
            this, [this]() { m_userDragging = true; });
    connect(m_progressSlider, &QSlider::sliderReleased, this, [this]() {
        m_userDragging = false;
        emit seekRequested(m_progressSlider->value());
    });

    // Volume
    connect(m_volumeSlider, &QSlider::valueChanged,
            m_audio, [this](int val) {
        m_audio->setVolume(static_cast<float>(val) / 100.0f);
    });
}

void PlayerControls::updatePlayButton() {
    if (m_audio->state() == QMediaPlayer::PlayingState) {
        m_playBtn->setText("⏸");
    } else {
        m_playBtn->setText("▶");
    }
}

void PlayerControls::updateTimeDisplay() {
    int64_t pos = m_audio->position();
    int64_t totalSec = pos / 1000;
    m_timeLabel->setText(
        QString("%1:%2").arg(totalSec / 60).arg(totalSec % 60, 2, 10, QChar('0')));
}
