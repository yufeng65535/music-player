#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCache>

class CoverArtService : public QObject {
    Q_OBJECT
public:
    explicit CoverArtService(QObject* parent = nullptr);

    // Fetch cover art for a track (two-step: MBID lookup -> image download)
    void fetchCoverArt(const QString& artist, const QString& album);

signals:
    void coverArtFetched(const QByteArray& imageData);
    void errorOccurred(const QString& message);

private:
    void onMusicBrainzReply(QNetworkReply* reply);
    void onCoverArchiveReply(QNetworkReply* reply);

    QNetworkAccessManager* m_nam = nullptr;
    QString m_pendingImageUrl;
};
