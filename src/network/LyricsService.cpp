#include "network/LyricsService.h"

#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

LyricsService::LyricsService(QObject* parent)
    : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
}

void LyricsService::fetchLyrics(const QString& title, const QString& artist,
                                 const QString& album) {
    QUrl url("https://lrclib.net/api/get");
    QUrlQuery params;
    params.addQueryItem("track_name", title);
    params.addQueryItem("artist_name", artist);
    if (!album.isEmpty()) {
        params.addQueryItem("album_name", album);
    }
    url.setQuery(params);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "MusicPlayer/0.1.0");

    QNetworkReply* reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred("LRCLIB error: " + reply->errorString());
            reply->deleteLater();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.isNull()) {
            emit errorOccurred("Invalid JSON from LRCLIB");
            reply->deleteLater();
            return;
        }

        QJsonObject obj = doc.object();
        QString plain = obj.value("plainLyrics").toString();
        QString synced = obj.value("syncedLyrics").toString();

        emit lyricsFetched(plain, synced);
        reply->deleteLater();
    });
}

void LyricsService::searchLyrics(const QString& query) {
    QUrl url("https://lrclib.net/api/search");
    QUrlQuery params;
    params.addQueryItem("q", query);
    url.setQuery(params);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "MusicPlayer/0.1.0");

    QNetworkReply* reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred("LRCLIB search error: " + reply->errorString());
            reply->deleteLater();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.isNull()) {
            emit errorOccurred("Invalid JSON from LRCLIB");
            reply->deleteLater();
            return;
        }

        QList<LyricsSearchResult> results;
        for (const auto& val : doc.array()) {
            QJsonObject obj = val.toObject();
            LyricsSearchResult r;
            r.id = obj.value("id").toVariant().toString();
            r.trackName = obj.value("trackName").toString();
            r.artistName = obj.value("artistName").toString();
            r.albumName = obj.value("albumName").toString();
            r.duration = obj.value("duration").toDouble();
            r.plainLyrics = obj.value("plainLyrics").toString();
            r.syncedLyrics = obj.value("syncedLyrics").toString();
            results.append(r);
        }

        emit searchResults(results);
        reply->deleteLater();
    });
}
