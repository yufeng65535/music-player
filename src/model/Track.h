#pragma once

#include <QString>
#include <QUrl>
#include <QDateTime>
#include <QByteArray>
#include <optional>
#include <cstdint>

struct Track {
    int64_t id = 0;

    // File info
    QString filePath;
    QUrl fileUrl;
    QString fileName;

    // Metadata
    QString title;
    QString artist;
    QString album;
    QString albumArtist;
    QString genre;
    QString composer;
    std::optional<uint16_t> trackNumber;
    std::optional<uint16_t> discNumber;
    std::optional<uint16_t> year;
    std::optional<uint32_t> durationMs;
    std::optional<uint32_t> bitrate;
    QString codec;

    // Cover art (raw JPEG/PNG bytes)
    QByteArray coverData;

    // Lyrics
    QString syncedLyrics;
    QString plainLyrics;

    // Library metadata
    QDateTime dateAdded;
    QDateTime dateModified;
    int playCount = 0;
    int rating = 0;
    QDateTime lastPlayed;
    bool isFavorited = false;

    // Derived display helpers
    QString displayTitle() const {
        return !title.isEmpty() ? title : fileName;
    }

    QString displayArtist() const {
        return !artist.isEmpty() ? artist : QStringLiteral("Unknown Artist");
    }

    QString displayAlbum() const {
        return !album.isEmpty() ? album : QStringLiteral("Unknown Album");
    }

    static QString formatDuration(uint32_t ms) {
        int totalSec = ms / 1000;
        int min = totalSec / 60;
        int sec = totalSec % 60;
        return QStringLiteral("%1:%2").arg(min, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0'));
    }

    QString formattedDuration() const {
        return durationMs ? formatDuration(*durationMs) : QStringLiteral("--:--");
    }

    bool operator==(const Track& other) const { return id == other.id; }
};

Q_DECLARE_METATYPE(Track)
Q_DECLARE_METATYPE(QList<Track>)
Q_DECLARE_METATYPE(std::optional<Track>)
