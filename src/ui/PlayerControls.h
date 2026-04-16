#pragma once

#include <QWidget>

class AudioEngine;
class QSlider;
class QPushButton;
class QLabel;

class PlayerControls : public QWidget {
    Q_OBJECT
public:
    explicit PlayerControls(AudioEngine* audio, QWidget* parent = nullptr);

signals:
    void seekRequested(int64_t positionMs);

private:
    void setupUI();
    void updatePlayButton();
    void updateTimeDisplay();

    AudioEngine* m_audio = nullptr;
    QPushButton* m_playBtn = nullptr;
    QSlider* m_progressSlider = nullptr;
    QSlider* m_volumeSlider = nullptr;
    QLabel* m_timeLabel = nullptr;
    QLabel* m_durationLabel = nullptr;
    bool m_userDragging = false;
};
