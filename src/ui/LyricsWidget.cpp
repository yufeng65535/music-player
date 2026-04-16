#include "ui/LyricsWidget.h"

#include <QVBoxLayout>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextEdit>
#include <QLabel>
#include <QScrollBar>
#include "core/LyricsManager.h"

LyricsWidget::LyricsWidget(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    m_titleLabel = new QLabel("Lyrics", this);
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
    layout->addWidget(m_titleLabel);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setPlaceholderText("No lyrics available");
    layout->addWidget(m_textEdit);
}

void LyricsWidget::setLyrics(const QString& syncedLrc, const QString& plainText) {
    m_plainLyrics = plainText;
    m_currentLine = -1;

    if (!syncedLrc.isEmpty()) {
        m_lines = LyricsManager::parseLrc(syncedLrc);
        if (!m_lines.isEmpty()) {
            m_textEdit->clear();
            for (const auto& line : m_lines) {
                m_textEdit->append(line.text);
            }
            m_textEdit->moveCursor(QTextCursor::Start);
            m_titleLabel->setText("Lyrics (synced)");
            return;
        }
    }

    if (!plainText.isEmpty()) {
        m_lines.clear();
        m_textEdit->setPlainText(plainText);
        m_textEdit->moveCursor(QTextCursor::Start);
        m_titleLabel->setText("Lyrics");
        return;
    }

    m_lines.clear();
    m_textEdit->clear();
    m_textEdit->setPlaceholderText("No lyrics available");
    m_titleLabel->setText("Lyrics");
}

void LyricsWidget::updatePosition(int64_t positionMs) {
    if (m_lines.isEmpty()) return;

    int newLine = LyricsManager::currentLineIndex(m_lines, positionMs);
    if (newLine == m_currentLine) return;

    m_currentLine = newLine;
    if (m_currentLine < 0) return;

    // Move to the line
    QTextBlock block = m_textEdit->document()->findBlockByLineNumber(m_currentLine);
    QTextCursor cursor(block);
    m_textEdit->setTextCursor(cursor);

    // Auto-scroll
    m_textEdit->ensureCursorVisible();
}
