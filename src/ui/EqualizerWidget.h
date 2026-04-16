#pragma once

#include <QWidget>
#include <array>

class AudioEngine;
class QSlider;
class QLabel;

class EqualizerWidget : public QWidget {
    Q_OBJECT
public:
    explicit EqualizerWidget(AudioEngine* audio, QWidget* parent = nullptr);

private:
    void setupUI();
    void onSliderChanged(int band, int value);

    AudioEngine* m_audio = nullptr;
    std::array<QSlider*, 10> m_sliders{};
    std::array<QLabel*, 10> m_labels{};
};
