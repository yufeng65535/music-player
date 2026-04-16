#pragma once

#include <QMainWindow>
#include "model/Track.h"

class QSplitter;
class QDockWidget;
class Application;
class AudioEngine;
class LibraryScanner;
class Database;
class LibraryPanel;
class PlayerControls;
class VisualizationWidget;
class EqualizerWidget;
class LyricsWidget;
class TagEditorDialog;
class PlaylistPanel;
class OnlineSearchDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(Application* app, QWidget* parent = nullptr);

private:
    void setupUI();
    void setupMenuBar();
    void setupConnections();

    Application* m_app = nullptr;
    AudioEngine* m_audio = nullptr;
    LibraryScanner* m_scanner = nullptr;

    // UI panels
    LibraryPanel* m_libraryPanel = nullptr;
    PlayerControls* m_playerControls = nullptr;
    VisualizationWidget* m_visualization = nullptr;
    EqualizerWidget* m_equalizer = nullptr;
    LyricsWidget* m_lyricsWidget = nullptr;
    PlaylistPanel* m_playlistPanel = nullptr;

    // Central widgets
    QSplitter* m_mainSplitter = nullptr;
    QSplitter* m_centerSplitter = nullptr;

    // Track list (current view)
    QList<Track> m_allTracks;
    QList<Track> m_currentQueue;
    int m_currentIndex = -1;

    // Music directories to scan
    QStringList m_musicDirs;

    // Action handlers
    void openMusicFolder();
    void playTrack(const Track& track);
    void playNext();
    void playPrevious();
    void showTagEditor(const QList<Track>& tracks);
    void showOnlineSearch();
};
