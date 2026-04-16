#pragma once

#include <QWidget>
#include <QList>
#include "core/LyricsManager.h"

class QTextEdit;
class QLabel;

class LyricsWidget : public QWidget {
    Q_OBJECT
public:
    explicit LyricsWidget(QWidget* parent = nullptr);

    void setLyrics(const QString& syncedLrc, const QString& plainText);

public slots:
    void updatePosition(int64_t positionMs);

private:
    QList<LrcLine> m_lines;
    QString m_plainLyrics;
    int m_currentLine = -1;

    QTextEdit* m_textEdit = nullptr;
    QLabel* m_titleLabel = nullptr;
};
