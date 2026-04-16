#pragma once

#include <QWidget>
#include <QList>
#include <QTimer>

class VisualizationWidget : public QWidget {
    Q_OBJECT
public:
    explicit VisualizationWidget(QWidget* parent = nullptr);

    enum Mode { Spectrum, Waveform };
    void setMode(Mode mode);

public slots:
    void setSpectrumData(const QList<float>& magnitudes);

protected:
    void paintEvent(QPaintEvent* event) override;
    void timerEvent(QTimerEvent* event) override;

private:
    Mode m_mode = Spectrum;
    QList<float> m_spectrumData;
    int m_timerId = 0;
};
