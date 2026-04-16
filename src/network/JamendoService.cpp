#include "network/JamendoService.h"

#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QDebug>

JamendoService::JamendoService(const QString& clientId, QObject* parent)
    : QObject(parent), m_clientId(clientId)
{
    m_nam = new QNetworkAccessManager(this);
}

void JamendoService::search(const QString& query, int limit) {
    QUrl url("https://api.jamendo.com/v3.0/tracks/");
    QUrlQuery params;
    params.addQueryItem("client_id", m_clientId);
    params.addQueryItem("format", "json");
    params.addQueryItem("search", query);
    params.addQueryItem("limit", QString::number(limit));
    params.addQueryItem("audioformat", "mp31");
    params.addQueryItem("include", "musicinfo");
    url.setQuery(params);

    QNetworkReply* reply = m_nam->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleSearchReply(reply);
        reply->deleteLater();
    });
}

void JamendoService::fetchPopular(int limit) {
    QUrl url("https://api.jamendo.com/v3.0/tracks/");
    QUrlQuery params;
    params.addQueryItem("client_id", m_clientId);
    params.addQueryItem("format", "json");
    params.addQueryItem("order", "popularity_month");
    params.addQueryItem("limit", QString::number(limit));
    params.addQueryItem("audioformat", "mp31");
    params.addQueryItem("include", "musicinfo");
    url.setQuery(params);

    QNetworkReply* reply = m_nam->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleSearchReply(reply);
        reply->deleteLater();
    });
}

void JamendoService::downloadTrack(const JamendoTrack& track, const QString& destinationDir) {
    if (!QDir().mkpath(destinationDir)) {
        emit errorOccurred("Cannot create download directory: " + destinationDir);
        return;
    }

    QNetworkRequest req(track.streamUrl);
    QNetworkReply* reply = m_nam->get(req);

    QString destPath = destinationDir + "/" + track.artist + " - " + track.title + ".mp3";
    destPath = destPath.replace(QRegularExpression("[<>:\"/\\\\|?*]"), "_");

    QFile* file = new QFile(destPath, this);
    if (!file->open(QIODevice::WriteOnly)) {
        emit errorOccurred("Cannot open file for writing: " + destPath);
        reply->deleteLater();
        delete file;
        return;
    }

    connect(reply, &QNetworkReply::readyRead, this, [reply, file]() {
        file->write(reply->readAll());
    });
    connect(reply, &QNetworkReply::downloadProgress, this,
            [this, &track](qint64 received, qint64 total) {
        if (total > 0) {
            emit downloadProgress(track.id, static_cast<float>(received) / total);
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, file, destPath]() {
        file->close();
        delete file;
        if (reply->error() == QNetworkReply::NoError) {
            emit downloadFinished(destPath);
        } else {
            emit errorOccurred("Download failed: " + reply->errorString());
        }
        reply->deleteLater();
    });
}

void JamendoService::handleSearchReply(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred("Jamendo API error: " + reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        emit errorOccurred("Invalid JSON from Jamendo API");
        return;
    }

    QJsonObject root = doc.object();
    QJsonObject resultObj = root.value("results").toObject();
    QJsonArray tracks = resultObj.value("results").toArray();

    QList<JamendoTrack> trackList;
    for (const auto& val : tracks) {
        QJsonObject obj = val.toObject();
        JamendoTrack t;
        t.id = obj.value("id").toString();
        t.title = obj.value("name").toString();
        t.artist = obj.value("artist_name").toString();
        t.album = obj.value("album_name").toString();
        t.durationSec = obj.value("duration").toInt();
        t.streamUrl = QUrl(obj.value("audio").toString());
        t.coverUrl = QUrl(obj.value("album_image").toString());
        t.licenseUrl = obj.value("license_ccurl").toString();

        QJsonArray tagsArr = obj.value("tags").toArray();
        for (const auto& tag : tagsArr) {
            t.tags.append(tag.toString());
        }

        trackList.append(t);
    }

    emit searchResults(trackList);
}
