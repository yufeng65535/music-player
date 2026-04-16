#include "db/Database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QDir>
#include <QVariant>

// Helper to avoid int64_t -> QVariant ambiguity
static inline QVariant v64(int64_t v) { return QVariant::fromValue<qlonglong>(v); }

static const char* kSchema[] = {
    "CREATE TABLE IF NOT EXISTS tracks ("
    "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "    file_path TEXT UNIQUE NOT NULL,"
    "    file_name TEXT,"
    "    title TEXT DEFAULT '',"
    "    artist TEXT DEFAULT '',"
    "    album TEXT DEFAULT '',"
    "    album_artist TEXT DEFAULT '',"
    "    genre TEXT DEFAULT '',"
    "    composer TEXT DEFAULT '',"
    "    track_number INTEGER,"
    "    disc_number INTEGER,"
    "    year INTEGER,"
    "    duration_ms INTEGER,"
    "    bitrate INTEGER,"
    "    codec TEXT DEFAULT '',"
    "    cover_data BLOB,"
    "    synced_lyrics TEXT DEFAULT '',"
    "    plain_lyrics TEXT DEFAULT '',"
    "    date_added DATETIME DEFAULT CURRENT_TIMESTAMP,"
    "    date_modified DATETIME,"
    "    play_count INTEGER DEFAULT 0,"
    "    rating INTEGER DEFAULT 0,"
    "    last_played DATETIME,"
    "    is_favorited INTEGER DEFAULT 0"
    ")",

    "CREATE TABLE IF NOT EXISTS playlists ("
    "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "    name TEXT NOT NULL,"
    "    description TEXT DEFAULT '',"
    "    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
    "    modified_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
    "    is_smart INTEGER DEFAULT 0,"
    "    smart_rule_json TEXT DEFAULT ''"
    ")",

    "CREATE TABLE IF NOT EXISTS playlist_tracks ("
    "    playlist_id INTEGER NOT NULL,"
    "    track_id INTEGER NOT NULL,"
    "    position INTEGER NOT NULL,"
    "    PRIMARY KEY (playlist_id, position),"
    "    FOREIGN KEY (playlist_id) REFERENCES playlists(id) ON DELETE CASCADE,"
    "    FOREIGN KEY (track_id) REFERENCES tracks(id) ON DELETE CASCADE"
    ")",

    "CREATE TABLE IF NOT EXISTS settings ("
    "    key TEXT PRIMARY KEY,"
    "    value TEXT NOT NULL"
    ")",

    "CREATE INDEX IF NOT EXISTS idx_tracks_artist ON tracks(artist)",
    "CREATE INDEX IF NOT EXISTS idx_tracks_album ON tracks(album)",
    "CREATE INDEX IF NOT EXISTS idx_tracks_title ON tracks(title)",
    "CREATE INDEX IF NOT EXISTS idx_tracks_date_added ON tracks(date_added)",
    nullptr
};

Database::Database(const QString& dbPath, QObject* parent)
    : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);
}

Database::~Database() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool Database::init() {
    if (!m_db.open()) {
        qCritical() << "Failed to open database:" << m_db.lastError();
        return false;
    }
    createSchema();
    return true;
}

void Database::createSchema() {
    QSqlQuery query(m_db);
    for (int i = 0; kSchema[i] != nullptr; ++i) {
        if (!query.exec(kSchema[i])) {
            qWarning() << "Schema statement" << i << "failed:" << query.lastError();
        }
    }
}

// ── Track helpers ─────────────────────────────────────────────────

static void bindTrack(QSqlQuery& q, const Track& t) {
    q.bindValue(":file_path", t.filePath);
    q.bindValue(":file_name", t.fileName);
    q.bindValue(":title", t.title);
    q.bindValue(":artist", t.artist);
    q.bindValue(":album", t.album);
    q.bindValue(":album_artist", t.albumArtist);
    q.bindValue(":genre", t.genre);
    q.bindValue(":composer", t.composer);
    q.bindValue(":track_number", t.trackNumber.has_value() ? *t.trackNumber : QVariant());
    q.bindValue(":disc_number", t.discNumber.has_value() ? *t.discNumber : QVariant());
    q.bindValue(":year", t.year.has_value() ? *t.year : QVariant());
    q.bindValue(":duration_ms", t.durationMs.has_value() ? *t.durationMs : QVariant());
    q.bindValue(":bitrate", t.bitrate.has_value() ? *t.bitrate : QVariant());
    q.bindValue(":codec", t.codec);
    q.bindValue(":cover_data", t.coverData);
    q.bindValue(":synced_lyrics", t.syncedLyrics);
    q.bindValue(":plain_lyrics", t.plainLyrics);
    q.bindValue(":date_modified", t.dateModified);
}

Track Database::rowToTrack(const QSqlQuery& q) const {
    Track t;
    t.id = q.value("id").toLongLong();
    t.filePath = q.value("file_path").toString();
    t.fileName = q.value("file_name").toString();
    t.fileUrl = QUrl::fromLocalFile(t.filePath);
    t.title = q.value("title").toString();
    t.artist = q.value("artist").toString();
    t.album = q.value("album").toString();
    t.albumArtist = q.value("album_artist").toString();
    t.genre = q.value("genre").toString();
    t.composer = q.value("composer").toString();
    if (!q.value("track_number").isNull())
        t.trackNumber = q.value("track_number").toUInt();
    if (!q.value("disc_number").isNull())
        t.discNumber = q.value("disc_number").toUInt();
    if (!q.value("year").isNull())
        t.year = q.value("year").toUInt();
    if (!q.value("duration_ms").isNull())
        t.durationMs = q.value("duration_ms").toUInt();
    if (!q.value("bitrate").isNull())
        t.bitrate = q.value("bitrate").toUInt();
    t.codec = q.value("codec").toString();
    t.coverData = q.value("cover_data").toByteArray();
    t.syncedLyrics = q.value("synced_lyrics").toString();
    t.plainLyrics = q.value("plain_lyrics").toString();
    t.dateAdded = q.value("date_added").toDateTime();
    t.dateModified = q.value("date_modified").toDateTime();
    t.playCount = q.value("play_count").toInt();
    t.rating = q.value("rating").toInt();
    t.lastPlayed = q.value("last_played").toDateTime();
    t.isFavorited = q.value("is_favorited").toBool();
    return t;
}

bool Database::insertTrack(const Track& track) {
    QSqlQuery q(m_db);
    q.prepare(R"(
        INSERT INTO tracks (
            file_path, file_name, title, artist, album, album_artist,
            genre, composer, track_number, disc_number, year,
            duration_ms, bitrate, codec, cover_data,
            synced_lyrics, plain_lyrics, date_modified,
            play_count, rating, last_played, is_favorited
        ) VALUES (
            :file_path, :file_name, :title, :artist, :album, :album_artist,
            :genre, :composer, :track_number, :disc_number, :year,
            :duration_ms, :bitrate, :codec, :cover_data,
            :synced_lyrics, :plain_lyrics, :date_modified,
            :play_count, :rating, :last_played, :is_favorited
        )
    )");
    bindTrack(q, track);
    if (!q.exec()) {
        qWarning() << "Insert track failed:" << q.lastError();
        return false;
    }
    return true;
}

bool Database::updateTrack(const Track& track) {
    QSqlQuery q(m_db);
    q.prepare(R"(
        UPDATE tracks SET
            file_name=:file_name, title=:title, artist=:artist,
            album=:album, album_artist=:album_artist,
            genre=:genre, composer=:composer,
            track_number=:track_number, disc_number=:disc_number,
            year=:year, duration_ms=:duration_ms, bitrate=:bitrate,
            codec=:codec, cover_data=:cover_data,
            synced_lyrics=:synced_lyrics, plain_lyrics=:plain_lyrics,
            date_modified=:date_modified, play_count=:play_count,
            rating=:rating, last_played=:last_played,
            is_favorited=:is_favorited
        WHERE id=:id
    )");
    bindTrack(q, track);
    q.bindValue(":id", v64(track.id));
    if (!q.exec()) {
        qWarning() << "Update track failed:" << q.lastError();
        return false;
    }
    return true;
}

bool Database::deleteTrack(int64_t id) {
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM tracks WHERE id = ?");
    q.addBindValue(v64(id));
    return q.exec();
}

std::optional<Track> Database::getTrack(int64_t id) const {
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM tracks WHERE id = ?");
    q.addBindValue(v64(id));
    if (q.exec() && q.next()) {
        return rowToTrack(q);
    }
    return std::nullopt;
}

QList<Track> Database::getAllTracks() const {
    QList<Track> tracks;
    QSqlQuery q("SELECT * FROM tracks ORDER BY artist, album, title", m_db);
    while (q.next()) {
        tracks.append(rowToTrack(q));
    }
    return tracks;
}

QList<Track> Database::getTracksByArtist(const QString& artist) const {
    QList<Track> tracks;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM tracks WHERE artist = ? ORDER BY album, title");
    q.addBindValue(artist);
    if (q.exec()) {
        while (q.next()) tracks.append(rowToTrack(q));
    }
    return tracks;
}

QList<Track> Database::getTracksByAlbum(const QString& album) const {
    QList<Track> tracks;
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM tracks WHERE album = ? ORDER BY track_number, title");
    q.addBindValue(album);
    if (q.exec()) {
        while (q.next()) tracks.append(rowToTrack(q));
    }
    return tracks;
}

QList<Track> Database::searchTracks(const QString& query) const {
    QList<Track> tracks;
    QSqlQuery q(m_db);
    QString pattern = "%" + query + "%";
    q.prepare(R"(
        SELECT * FROM tracks
        WHERE title LIKE :p OR artist LIKE :p OR album LIKE :p
        ORDER BY artist, album, title
    )");
    q.bindValue(":p", pattern);
    if (q.exec()) {
        while (q.next()) tracks.append(rowToTrack(q));
    }
    return tracks;
}

bool Database::trackExists(const QString& filePath) const {
    QSqlQuery q(m_db);
    q.prepare("SELECT 1 FROM tracks WHERE file_path = ?");
    q.addBindValue(filePath);
    return q.exec() && q.next();
}

// ── Playlist helpers ──────────────────────────────────────────────

Playlist Database::rowToPlaylist(const QSqlQuery& q) const {
    Playlist p;
    p.id = q.value("id").toLongLong();
    p.name = q.value("name").toString();
    p.description = q.value("description").toString();
    p.createdAt = q.value("created_at").toDateTime();
    p.modifiedAt = q.value("modified_at").toDateTime();
    p.isSmart = q.value("is_smart").toBool();
    p.smartRuleJson = q.value("smart_rule_json").toString();

    QSqlQuery tq(m_db);
    tq.prepare("SELECT track_id FROM playlist_tracks WHERE playlist_id = ? ORDER BY position");
    tq.addBindValue(v64(p.id));
    if (tq.exec()) {
        while (tq.next()) {
            p.trackIds.append(tq.value(0).toLongLong());
        }
    }
    return p;
}

int64_t Database::createPlaylist(const QString& name, const QString& description) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO playlists (name, description) VALUES (?, ?)");
    q.addBindValue(name);
    q.addBindValue(description);
    if (q.exec()) {
        q.exec("SELECT last_insert_rowid()");
        if (q.next()) return q.value(0).toLongLong();
    }
    return -1;
}

bool Database::deletePlaylist(int64_t id) {
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM playlists WHERE id = ?");
    q.addBindValue(v64(id));
    return q.exec();
}

bool Database::renamePlaylist(int64_t id, const QString& name) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE playlists SET name = ?, modified_at = CURRENT_TIMESTAMP WHERE id = ?");
    q.addBindValue(name);
    q.addBindValue(v64(id));
    return q.exec();
}

std::optional<Playlist> Database::getPlaylist(int64_t id) const {
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM playlists WHERE id = ?");
    q.addBindValue(v64(id));
    if (q.exec() && q.next()) {
        return rowToPlaylist(q);
    }
    return std::nullopt;
}

QList<Playlist> Database::getAllPlaylists() const {
    QList<Playlist> playlists;
    QSqlQuery q("SELECT * FROM playlists ORDER BY name", m_db);
    while (q.next()) {
        playlists.append(rowToPlaylist(q));
    }
    return playlists;
}

bool Database::addTrackToPlaylist(int64_t playlistId, int64_t trackId) {
    QSqlQuery q(m_db);
    q.prepare("SELECT COALESCE(MAX(position), -1) + 1 FROM playlist_tracks WHERE playlist_id = ?");
    q.addBindValue(v64(playlistId));
    if (q.exec() && q.next()) {
        int pos = q.value(0).toInt();
        QSqlQuery ins(m_db);
        ins.prepare("INSERT INTO playlist_tracks (playlist_id, track_id, position) VALUES (?, ?, ?)");
        ins.addBindValue(v64(playlistId));
        ins.addBindValue(v64(trackId));
        ins.addBindValue(pos);
        if (ins.exec()) {
            QSqlQuery upd(m_db);
            upd.prepare("UPDATE playlists SET modified_at = CURRENT_TIMESTAMP WHERE id = ?");
            upd.addBindValue(v64(playlistId));
            upd.exec();
            return true;
        }
    }
    return false;
}

bool Database::removeTrackFromPlaylist(int64_t playlistId, int64_t trackId) {
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM playlist_tracks WHERE playlist_id = ? AND track_id = ?");
    q.addBindValue(v64(playlistId));
    q.addBindValue(v64(trackId));
    return q.exec();
}

bool Database::setPlaylistTracks(int64_t playlistId, const QList<int64_t>& trackIds) {
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM playlist_tracks WHERE playlist_id = ?");
    q.addBindValue(v64(playlistId));
    if (!q.exec()) return false;

    for (int i = 0; i < trackIds.size(); ++i) {
        QSqlQuery ins(m_db);
        ins.prepare("INSERT INTO playlist_tracks (playlist_id, track_id, position) VALUES (?, ?, ?)");
        ins.addBindValue(v64(playlistId));
        ins.addBindValue(v64(trackIds[i]));
        ins.addBindValue(i);
        if (!ins.exec()) return false;
    }

    QSqlQuery upd(m_db);
    upd.prepare("UPDATE playlists SET modified_at = CURRENT_TIMESTAMP WHERE id = ?");
    upd.addBindValue(v64(playlistId));
    return upd.exec();
}

// ── Settings ──────────────────────────────────────────────────────

void Database::setSetting(const QString& key, const QString& value) {
    QSqlQuery q(m_db);
    q.prepare("INSERT OR REPLACE INTO settings (key, value) VALUES (?, ?)");
    q.addBindValue(key);
    q.addBindValue(value);
    q.exec();
}

QString Database::getSetting(const QString& key, const QString& defaultVal) const {
    QSqlQuery q(m_db);
    q.prepare("SELECT value FROM settings WHERE key = ?");
    q.addBindValue(key);
    if (q.exec() && q.next()) {
        return q.value(0).toString();
    }
    return defaultVal;
}
