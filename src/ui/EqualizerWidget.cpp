#include "ui/EqualizerWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include "core/AudioEngine.h"

static const char* kBandLabels[] = {
    "31", "62", "125", "250", "500", "1K", "2K", "4K", "8K", "16K"
};

// Presets: {31, 62, 125, 250, 500, 1K, 2K, 4K, 8K, 16K}
static const int kPresets[][10] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},       // Flat
    {8, 6, 4, 2, 0, 0, 0, 0, 2, 4},        // Bass Boost
    {0, 0, 0, 0, 2, 4, 6, 8, 8, 8},        // Treble Boost
    {5, 4, 2, 0, -2, -2, 0, 3, 5, 6},      // Rock
    {-1, 2, 4, 5, 4, 2, 0, -1, -1, 0},     // Pop
    {4, 3, 1, 0, -1, -2, 0, 2, 4, 5},      // Jazz
    {6, 4, 2, 0, -2, -2, 0, 2, 4, 6},      // Classical
};

static const char* kPresetNames[] = {
    "Flat", "Bass Boost", "Treble Boost", "Rock", "Pop", "Jazz", "Classical"
};

EqualizerWidget::EqualizerWidget(AudioEngine* audio, QWidget* parent)
    : QWidget(parent), m_audio(audio)
{
    setupUI();
}

void EqualizerWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    // Header
    auto* header = new QHBoxLayout();
    header->addWidget(new QLabel("Equalizer", this));

    auto* presetCombo = new QComboBox(this);
    for (const auto* name : kPresetNames) {
        presetCombo->addItem(name);
    }
    header->addWidget(presetCombo);
    layout->addLayout(header);

    // EQ sliders
    auto* sliderLayout = new QHBoxLayout();
    sliderLayout->setSpacing(8);

    for (int i = 0; i < 10; ++i) {
        auto* bandWidget = new QWidget(this);
        auto* bandLayout = new QVBoxLayout(bandWidget);
        bandLayout->setContentsMargins(0, 0, 0, 0);
        bandLayout->setSpacing(2);

        // Gain label
        m_labels[i] = new QLabel("0 dB", bandWidget);
        m_labels[i]->setAlignment(Qt::AlignCenter);
        m_labels[i]->setFixedWidth(40);
        bandLayout->addWidget(m_labels[i]);

        // Slider
        m_sliders[i] = new QSlider(Qt::Vertical, bandWidget);
        m_sliders[i]->setRange(-12, 12);
        m_sliders[i]->setValue(0);
        m_sliders[i]->setFixedHeight(120);
        m_sliders[i]->setFixedWidth(30);
        bandLayout->addWidget(m_sliders[i], 0, Qt::AlignCenter);

        // Frequency label
        auto* freqLabel = new QLabel(kBandLabels[i], bandWidget);
        freqLabel->setAlignment(Qt::AlignCenter);
        freqLabel->setFixedWidth(40);
        bandLayout->addWidget(freqLabel);

        sliderLayout->addWidget(bandWidget);

        // Slider changed
        int band = i;
        connect(m_sliders[i], &QSlider::valueChanged, this,
                [this, band](int val) { onSliderChanged(band, val); });
    }

    layout->addLayout(sliderLayout);

    // Enable toggle
    auto* enableBtn = new QPushButton("EQ: OFF", this);
    enableBtn->setCheckable(true);
    connect(enableBtn, &QPushButton::toggled, this, [this, enableBtn](bool checked) {
        m_audio->setEqEnabled(checked);
        enableBtn->setText(checked ? "EQ: ON" : "EQ: OFF");
    });
    layout->addWidget(enableBtn);

    // Preset selection
    connect(presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        if (index < 0 || index >= static_cast<int>(std::size(kPresets))) return;
        for (int i = 0; i < 10; ++i) {
            m_sliders[i]->setValue(kPresets[index][i]);
        }
    });
}

void EqualizerWidget::onSliderChanged(int band, int value) {
    m_audio->setEqBand(band, static_cast<float>(value));
    m_labels[band]->setText(QString("%1 dB").arg(value));
}
