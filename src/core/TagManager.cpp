#include "core/TagManager.h"

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/flacfile.h>
#include <taglib/flacpicture.h>
#include <taglib/mp4file.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4coverart.h>
#include <taglib/taglib.h>

#include <QFileInfo>
#include <QDebug>
#include <optional>

static const QStringList kSupportedExts = {
    ".mp3", ".flac", ".ogg", ".m4a", ".aac", ".wav", ".wma", ".opus"
};

bool TagManager::isSupported(const QString& filePath) {
    QString ext = QFileInfo(filePath).suffix().toLower();
    return kSupportedExts.contains("." + ext);
}

QStringList TagManager::supportedExtensions() {
    return kSupportedExts;
}

template<typename T>
static std::optional<T> optIf(T val, T empty) {
    return (val != empty) ? std::optional<T>(val) : std::nullopt;
}

Track TagManager::readTags(const QString& filePath) {
    Track track;
    track.filePath = filePath;
    track.fileUrl = QUrl::fromLocalFile(filePath);
    track.fileName = QFileInfo(filePath).fileName();

    TagLib::FileRef ref(filePath.toUtf8().constData(), true);
    if (ref.isNull() || ref.tag() == nullptr) {
        return track;
    }

    TagLib::Tag* tag = ref.tag();
    track.title = QString::fromStdString(tag->title().to8Bit(true));
    track.artist = QString::fromStdString(tag->artist().to8Bit(true));
    track.album = QString::fromStdString(tag->album().to8Bit(true));
    track.genre = QString::fromStdString(tag->genre().to8Bit(true));
    track.year = optIf(static_cast<uint16_t>(tag->year()), static_cast<uint16_t>(0));
    track.trackNumber = optIf(static_cast<uint16_t>(tag->track()), static_cast<uint16_t>(0));

    if (tag->track() != 0) {
        track.trackNumber = static_cast<uint16_t>(tag->track());
    }

    // Duration from audio properties
    if (ref.audioProperties()) {
        track.durationMs = static_cast<uint32_t>(ref.audioProperties()->lengthInMilliseconds());
        track.bitrate = static_cast<uint32_t>(ref.audioProperties()->bitrate());
    }

    // Try to extract cover art
    try {
        // MP3 (ID3v2)
        TagLib::MPEG::File mpeg(filePath.toUtf8().constData());
        if (mpeg.isValid() && mpeg.ID3v2Tag()) {
            auto& frames = mpeg.ID3v2Tag()->frameListMap()["APIC"];
            if (!frames.isEmpty()) {
                TagLib::ID3v2::AttachedPictureFrame* pic =
                    static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frames.front());
                track.coverData = QByteArray(
                    reinterpret_cast<const char*>(pic->picture().data()),
                    static_cast<int>(pic->picture().size()));
            }
        }

        // FLAC
        if (track.coverData.isEmpty()) {
            TagLib::FLAC::File flac(filePath.toUtf8().constData(),
                                     TagLib::ID3v2::FrameFactory::instance(), true);
            if (flac.isValid()) {
                auto pics = flac.pictureList();
                if (!pics.isEmpty()) {
                    auto pic = pics.front();
                    track.coverData = QByteArray(
                        reinterpret_cast<const char*>(pic->data().data()),
                        static_cast<int>(pic->data().size()));
                }
            }
        }

        // MP4/M4A
        if (track.coverData.isEmpty()) {
            TagLib::MP4::File mp4(filePath.toUtf8().constData());
            if (mp4.isValid()) {
#if TAGLIB_MAJOR_VERSION >= 2
                // TagLib 2.x: use itemMap() + item() + toCoverArtList()
                if (mp4.tag()->itemMap().contains("covr")) {
                    auto coverArtList = mp4.tag()->item("covr").toCoverArtList();
                    if (!coverArtList.isEmpty()) {
                        auto img = coverArtList.front();
                        auto bv = img.data();
                        track.coverData = QByteArray(
                            reinterpret_cast<const char*>(bv.data()),
                            static_cast<int>(bv.size()));
                    }
                }
#else
                // TagLib 1.x: use itemListMap()
                auto& items = mp4.tag()->itemListMap();
                if (items.contains("covr")) {
                    auto coverArtList = items["covr"].toCoverArtList();
                    if (!coverArtList.isEmpty()) {
                        auto img = coverArtList.front();
                        track.coverData = QByteArray(
                            reinterpret_cast<const char*>(img.data().data()),
                            static_cast<int>(img.data().size()));
                    }
                }
#endif
            }
        }
    } catch (const std::exception& e) {
        qWarning() << "Error extracting cover art:" << QString::fromUtf8(e.what());
    }

    // Detect codec from extension
    QString ext = QFileInfo(filePath).suffix().toLower();
    track.codec = ext;

    return track;
}

bool TagManager::writeTags(const QString& filePath, const Track& track) {
    TagLib::FileRef ref(filePath.toUtf8().constData(), true);
    if (ref.isNull() || ref.tag() == nullptr) {
        qWarning() << "Cannot open file for tag writing:" << filePath;
        return false;
    }

    TagLib::Tag* tag = ref.tag();
    tag->setTitle(track.title.toUtf8().constData());
    tag->setArtist(track.artist.toUtf8().constData());
    tag->setAlbum(track.album.toUtf8().constData());
    tag->setGenre(track.genre.toUtf8().constData());
    if (track.year) tag->setYear(*track.year);
    if (track.trackNumber) tag->setTrack(*track.trackNumber);

    if (!ref.save()) {
        qWarning() << "Failed to save tags for:" << filePath;
        return false;
    }

    return true;
}
