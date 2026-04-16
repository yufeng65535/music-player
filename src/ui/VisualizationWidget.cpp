#include "ui/VisualizationWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QTimerEvent>
#include <cmath>

VisualizationWidget::VisualizationWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(80);
    m_timerId = startTimer(33);  // ~30fps
}

void VisualizationWidget::setMode(Mode mode) {
    m_mode = mode;
    update();
}

void VisualizationWidget::setSpectrumData(const QList<float>& magnitudes) {
    m_spectrumData = magnitudes;
    update();
}

void VisualizationWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background
    painter.fillRect(rect(), QColor(0x16, 0x21, 0x3e));

    if (m_spectrumData.isEmpty()) return;

    int w = width();
    int h = height();

    if (m_mode == Spectrum) {
        // ── Spectrum analyzer ──────────────────────────────
        int barCount = qMin(64, m_spectrumData.size());
        int barWidth = (w - barCount) / barCount;
        int gap = 2;

        QLinearGradient grad(0, h, 0, 0);
        grad.setColorAt(0.0, QColor(0xe9, 0x45, 0x60));
        grad.setColorAt(0.5, QColor(0x0f, 0x34, 0x60));
        grad.setColorAt(1.0, QColor(0x53, 0x34, 0x83));

        painter.setBrush(grad);
        painter.setPen(Qt::NoPen);

        for (int i = 0; i < barCount; ++i) {
            // Log-scale the magnitude
            float val = m_spectrumData[i];
            float normalized = std::log10(1.0f + val * 100.0f) / 2.0f;
            normalized = qBound(0.0f, normalized, 1.0f);

            int barH = static_cast<int>(normalized * (h - 10));
            int x = i * (barWidth + gap) + gap;
            int y = h - barH;

            painter.drawRoundedRect(x, y, barWidth, barH, 2, 2);
        }
    } else {
        // ── Waveform ───────────────────────────────────────
        painter.setPen(QPen(QColor(0xe9, 0x45, 0x60), 2));

        QPainterPath path;
        int step = qMax(1, m_spectrumData.size() / w);

        for (int i = 0; i < m_spectrumData.size() && i * step < w; ++i) {
            float val = m_spectrumData[i] - 0.5f;  // Center around 0
            int x = i * step;
            int y = static_cast<int>(h / 2 + val * h * 0.8f);

            if (i == 0) {
                path.moveTo(x, y);
            } else {
                path.lineTo(x, y);
            }
        }

        painter.drawPath(path);
    }
}

void VisualizationWidget::timerEvent(QTimerEvent* event) {
    if (event->timerId() == m_timerId) {
        update();
    }
}
