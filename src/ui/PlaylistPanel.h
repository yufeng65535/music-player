#pragma once

#include <QWidget>
#include <QList>
#include "model/Playlist.h"

class Database;
class QListWidget;
class QPushButton;
class QLineEdit;

class PlaylistPanel : public QWidget {
    Q_OBJECT
public:
    explicit PlaylistPanel(Database* db, QWidget* parent = nullptr);

signals:
    void playlistSelected(int64_t playlistId);

private:
    void setupUI();
    void refreshPlaylists();
    void createNewPlaylist();
    void deleteSelectedPlaylist();
    void loadSelectedPlaylist();

    Database* m_db = nullptr;
    QListWidget* m_listWidget = nullptr;
    QLineEdit* m_nameEdit = nullptr;
    QList<Playlist> m_playlists;
};
