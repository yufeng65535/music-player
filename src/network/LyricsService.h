#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

struct LyricsSearchResult {
    QString id;
    QString trackName;
    QString artistName;
    QString albumName;
    double duration = 0;
    QString plainLyrics;
    QString syncedLyrics;
};

class LyricsService : public QObject {
    Q_OBJECT
public:
    explicit LyricsService(QObject* parent = nullptr);

    // Fetch lyrics by track metadata
    void fetchLyrics(const QString& title, const QString& artist,
                     const QString& album = {});

    // Full-text search
    void searchLyrics(const QString& query);

signals:
    void lyricsFetched(const QString& plainLyrics, const QString& syncedLyrics);
    void searchResults(const QList<LyricsSearchResult>& results);
    void errorOccurred(const QString& message);

private:
    QNetworkAccessManager* m_nam = nullptr;
};
