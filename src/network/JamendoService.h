#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QList>

struct JamendoTrack {
    QString id;
    QString title;
    QString artist;
    QString album;
    int durationSec = 0;
    QUrl streamUrl;
    QUrl coverUrl;
    QString licenseUrl;
    QStringList tags;
};

class JamendoService : public QObject {
    Q_OBJECT
public:
    explicit JamendoService(const QString& clientId, QObject* parent = nullptr);

    void search(const QString& query, int limit = 50);
    void fetchPopular(int limit = 50);
    void downloadTrack(const JamendoTrack& track, const QString& destinationDir);

signals:
    void searchResults(const QList<JamendoTrack>& tracks);
    void downloadProgress(const QString& trackId, float progress);
    void downloadFinished(const QString& filePath);
    void errorOccurred(const QString& message);

private:
    void handleSearchReply(QNetworkReply* reply);
    void handleDownloadReply(QNetworkReply* reply, const QString& destDir);

    QString m_clientId;
    QNetworkAccessManager* m_nam = nullptr;
};
