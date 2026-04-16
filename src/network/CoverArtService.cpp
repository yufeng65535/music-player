#include "network/CoverArtService.h"

#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QUrl>

CoverArtService::CoverArtService(QObject* parent)
    : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
}

void CoverArtService::fetchCoverArt(const QString& artist, const QString& album) {
    if (artist.isEmpty() && album.isEmpty()) {
        emit errorOccurred("Artist and album are both empty");
        return;
    }

    // Step 1: Search MusicBrainz for the release
    QString query = QString("artist:\"%1\" AND release:\"%2\"")
        .arg(artist, album);

    QUrl url("https://musicbrainz.org/ws/2/release/");
    QUrlQuery params;
    params.addQueryItem("query", query);
    params.addQueryItem("fmt", "json");
    params.addQueryItem("limit", "1");
    url.setQuery(params);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "MusicPlayer/0.1.0 (https://github.com/musicplayer)");

    QNetworkReply* reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onMusicBrainzReply(reply);
        reply->deleteLater();
    });
}

void CoverArtService::onMusicBrainzReply(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred("MusicBrainz error: " + reply->errorString());
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (doc.isNull()) {
        emit errorOccurred("Invalid JSON from MusicBrainz");
        return;
    }

    QJsonObject root = doc.object();
    QJsonArray releases = root.value("releases").toArray();
    if (releases.isEmpty()) {
        emit errorOccurred("No release found on MusicBrainz");
        return;
    }

    QString mbid = releases[0].toObject().value("id").toString();
    if (mbid.isEmpty()) {
        emit errorOccurred("No MBID found for release");
        return;
    }

    // Step 2: Query Cover Art Archive
    QUrl coverUrl("https://coverartarchive.org/release/" + mbid);

    QNetworkRequest req(coverUrl);
    req.setRawHeader("User-Agent", "MusicPlayer/0.1.0 (https://github.com/musicplayer)");

    QNetworkReply* coverReply = m_nam->get(req);
    connect(coverReply, &QNetworkReply::finished, this, [this, coverReply]() {
        onCoverArchiveReply(coverReply);
        coverReply->deleteLater();
    });
}

void CoverArtService::onCoverArchiveReply(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred("Cover Art Archive error: " + reply->errorString());
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (doc.isNull()) {
        emit errorOccurred("Invalid JSON from Cover Art Archive");
        return;
    }

    QJsonObject root = doc.object();
    QJsonArray images = root.value("images").toArray();

    // Find the first front image
    QString imageUrl;
    for (const auto& img : images) {
        QJsonObject obj = img.toObject();
        if (obj.value("front").toBool()) {
            imageUrl = obj.value("image").toString();
            break;
        }
    }

    if (imageUrl.isEmpty() && !images.isEmpty()) {
        imageUrl = images[0].toObject().value("image").toString();
    }

    if (imageUrl.isEmpty()) {
        emit errorOccurred("No cover art found");
        return;
    }

    // Step 3: Download the image
    QNetworkRequest imgReq{QUrl(imageUrl)};
    QNetworkReply* imgReply = m_nam->get(imgReq);
    connect(imgReply, &QNetworkReply::finished, this, [this, imgReply]() {
        if (imgReply->error() == QNetworkReply::NoError) {
            emit coverArtFetched(imgReply->readAll());
        } else {
            emit errorOccurred("Failed to download cover image: " + imgReply->errorString());
        }
        imgReply->deleteLater();
    });
}
