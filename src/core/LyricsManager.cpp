#include "core/LyricsManager.h"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>
#include <QDebug>

QList<LrcLine> LyricsManager::parseLrc(const QString& lrcContent) {
    QList<LrcLine> lines;

    // Match [mm:ss.xx] or [mm:ss.xxx]
    QRegularExpression re(R"(\[(\d+):(\d+)(?:\.(\d+))?\])");

    for (const QString& rawLine : lrcContent.split('\n')) {
        QString line = rawLine.trimmed();
        if (line.isEmpty()) continue;

        auto it = re.globalMatch(line);
        while (it.hasNext()) {
            auto match = it.next();
            int min = match.captured(1).toInt();
            int sec = match.captured(2).toInt();
            int ms = 0;
            if (match.lastCapturedIndex() >= 3) {
                QString frac = match.captured(3);
                // Normalize to 3 digits
                while (frac.length() < 3) frac.append('0');
                if (frac.length() > 3) frac = frac.left(3);
                ms = frac.toInt();
            }
            int64_t timestamp = (min * 60 + sec) * 1000 + ms;

            // Text after the last tag
            QString text = line.mid(match.capturedEnd()).trimmed();
            if (!text.isEmpty()) {
                lines.append({timestamp, text});
            }
        }
    }

    // Sort by timestamp
    std::sort(lines.begin(), lines.end(),
              [](const LrcLine& a, const LrcLine& b) {
                  return a.timestampMs < b.timestampMs;
              });

    return lines;
}

QString LyricsManager::findLocalLrc(const QString& audioFilePath) {
    QFileInfo fi(audioFilePath);
    QString lrcPath = fi.absolutePath() + "/" + fi.completeBaseName() + ".lrc";

    if (QFile::exists(lrcPath)) {
        return lrcPath;
    }

    return {};
}

int LyricsManager::currentLineIndex(const QList<LrcLine>& lines, int64_t positionMs) {
    int idx = -1;
    for (int i = 0; i < lines.size(); ++i) {
        if (lines[i].timestampMs <= positionMs) {
            idx = i;
        } else {
            break;
        }
    }
    return idx;
}

QString LyricsManager::lrcToPlain(const QString& lrcContent) {
    QList<LrcLine> lines = parseLrc(lrcContent);
    QStringList texts;
    for (const auto& line : lines) {
        texts.append(line.text);
    }
    return texts.join('\n');
}
