#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include "model/Track.h"
#include "model/Playlist.h"

class Database : public QObject {
    Q_OBJECT
public:
    explicit Database(const QString& dbPath, QObject* parent = nullptr);
    ~Database() override;

    bool init();

    // Track CRUD
    bool insertTrack(const Track& track);
    bool updateTrack(const Track& track);
    bool deleteTrack(int64_t id);
    std::optional<Track> getTrack(int64_t id) const;
    QList<Track> getAllTracks() const;
    QList<Track> getTracksByArtist(const QString& artist) const;
    QList<Track> getTracksByAlbum(const QString& album) const;
    QList<Track> searchTracks(const QString& query) const;
    bool trackExists(const QString& filePath) const;

    // Playlist CRUD
    int64_t createPlaylist(const QString& name, const QString& description = {});
    bool deletePlaylist(int64_t id);
    bool renamePlaylist(int64_t id, const QString& name);
    std::optional<Playlist> getPlaylist(int64_t id) const;
    QList<Playlist> getAllPlaylists() const;
    bool addTrackToPlaylist(int64_t playlistId, int64_t trackId);
    bool removeTrackFromPlaylist(int64_t playlistId, int64_t trackId);
    bool setPlaylistTracks(int64_t playlistId, const QList<int64_t>& trackIds);

    // Settings
    void setSetting(const QString& key, const QString& value);
    QString getSetting(const QString& key, const QString& defaultVal = {}) const;

private:
    Track rowToTrack(const QSqlQuery& query) const;
    Playlist rowToPlaylist(const QSqlQuery& query) const;
    void createSchema();

    QSqlDatabase m_db;
};
