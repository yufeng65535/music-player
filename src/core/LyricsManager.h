#pragma once

#include <QString>
#include <QList>

struct LrcLine {
    int64_t timestampMs;  // offset from start of track
    QString text;
};

class LyricsManager {
public:
    // Parse LRC content into timed lines
    static QList<LrcLine> parseLrc(const QString& lrcContent);

    // Find .lrc file next to the audio file (same name, .lrc extension)
    static QString findLocalLrc(const QString& audioFilePath);

    // Get current lyric line index for a given playback position
    static int currentLineIndex(const QList<LrcLine>& lines, int64_t positionMs);

    // Strip LRC time tags, return plain text
    static QString lrcToPlain(const QString& lrcContent);
};
