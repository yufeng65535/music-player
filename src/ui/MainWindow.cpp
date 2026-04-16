#include "ui/MainWindow.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QSettings>
#include <QDockWidget>
#include <QTableView>
#include <QDebug>

#include "app/Application.h"
#include "core/AudioEngine.h"
#include "core/LibraryScanner.h"
#include "ui/LibraryPanel.h"
#include "ui/PlayerControls.h"
#include "ui/VisualizationWidget.h"
#include "ui/EqualizerWidget.h"
#include "ui/LyricsWidget.h"
#include "ui/PlaylistPanel.h"
#include "ui/OnlineSearchDialog.h"
#include "ui/TagEditorDialog.h"

MainWindow::MainWindow(Application* app, QWidget* parent)
    : QMainWindow(parent), m_app(app)
{
    setWindowTitle("MusicPlayer");
    setMinimumSize(900, 600);

    m_audio = new AudioEngine(this);
    m_scanner = new LibraryScanner(app->database(), this);

    setupUI();
    setupMenuBar();
    setupConnections();

    // Restore music directories
    QSettings* settings = app->settings();
    m_musicDirs = settings->value("musicDirectories").toStringList();
}

void MainWindow::setupUI() {
    // ── Central area ────────────────────────────────────────
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);

    // Left: Library panel
    m_libraryPanel = new LibraryPanel(m_app->database(), m_mainSplitter);
    m_mainSplitter->addWidget(m_libraryPanel);

    // Center: Track list + visualization
    m_centerSplitter = new QSplitter(Qt::Vertical, m_mainSplitter);

    auto* centerWidget = new QWidget(m_centerSplitter);
    auto* centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->setContentsMargins(0, 0, 0, 0);

    // Library view (track table) - reuse library panel's table
    centerLayout->addWidget(m_libraryPanel->trackTable());

    // Visualization
    m_visualization = new VisualizationWidget(centerWidget);
    m_visualization->setFixedHeight(120);
    centerLayout->addWidget(m_visualization);

    m_centerSplitter->addWidget(centerWidget);

    // Right: Details panel (playlist + lyrics)
    auto* rightPanel = new QDockWidget("Details", this);
    rightPanel->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    rightPanel->setAllowedAreas(Qt::RightDockWidgetArea);

    auto* rightWidget = new QWidget(rightPanel);
    auto* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(4, 4, 4, 4);

    // Playlist panel
    m_playlistPanel = new PlaylistPanel(m_app->database(), rightWidget);
    rightLayout->addWidget(m_playlistPanel);

    // Lyrics widget
    m_lyricsWidget = new LyricsWidget(rightWidget);
    rightLayout->addWidget(m_lyricsWidget);

    rightPanel->setWidget(rightWidget);
    addDockWidget(Qt::RightDockWidgetArea, rightPanel);

    m_mainSplitter->setStretchFactor(0, 1);  // Left panel
    m_mainSplitter->setStretchFactor(1, 3);  // Center
    setCentralWidget(m_mainSplitter);

    // ── Bottom: Player controls ─────────────────────────────
    m_playerControls = new PlayerControls(m_audio, this);
    statusBar()->addPermanentWidget(m_playerControls, 1);

    // Status bar info
    statusBar()->showMessage("Ready - Open a music folder to get started");
}

void MainWindow::setupMenuBar() {
    auto* menuBar = this->menuBar();

    // File menu
    auto* fileMenu = menuBar->addMenu("File");

    auto* openAction = fileMenu->addAction("Open Music Folder...");
    openAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    connect(openAction, &QAction::triggered, this, &MainWindow::openMusicFolder);

    fileMenu->addSeparator();

    auto* onlineAction = fileMenu->addAction("Online Search...");
    onlineAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));
    connect(onlineAction, &QAction::triggered, this, &MainWindow::showOnlineSearch);

    fileMenu->addSeparator();

    auto* exitAction = fileMenu->addAction("Exit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // View menu
    auto* viewMenu = menuBar->addMenu("View");

    auto* toggleEQ = viewMenu->addAction("Equalizer");
    toggleEQ->setCheckable(true);
    connect(toggleEQ, &QAction::toggled, this, [this](bool checked) {
        m_equalizer = m_equalizer ? nullptr : m_equalizer;
        // Toggle EQ visibility (placeholder - EQ widget created on demand)
    });

    auto* toggleLyrics = viewMenu->addAction("Lyrics Panel");
    toggleLyrics->setCheckable(true);
    toggleLyrics->setChecked(true);

    // Help menu
    auto* helpMenu = menuBar->addMenu("Help");
    helpMenu->addAction("About")->setShortcut(QKeySequence(Qt::Key_F1));
}

void MainWindow::setupConnections() {
    // Library panel: double-click to play
    connect(m_libraryPanel, &LibraryPanel::trackActivated,
            this, [this](const Track& track) {
        playTrack(track);
    });

    // Library panel: search
    connect(m_libraryPanel, &LibraryPanel::searchChanged,
            this, [this](const QString& query) {
        if (query.isEmpty()) {
            m_libraryPanel->setTracks(m_allTracks);
        } else {
            m_libraryPanel->filterTracks(query);
        }
    });

    // Audio engine: spectrum data -> visualization
    connect(m_audio, &AudioEngine::spectrumDataReady,
            m_visualization, &VisualizationWidget::setSpectrumData);

    // Audio engine: position -> lyrics sync
    connect(m_audio, &AudioEngine::positionChanged,
            m_lyricsWidget, &LyricsWidget::updatePosition);

    // Audio engine: playback finished -> auto next
    connect(m_audio, &AudioEngine::playbackFinished,
            this, &MainWindow::playNext);

    // Scanner progress
    connect(m_scanner, &LibraryScanner::scanProgress,
            this, [this](int current, int total) {
        statusBar()->showMessage(
            QString("Scanning... %1 / %2").arg(current).arg(total));
    });

    connect(m_scanner, &LibraryScanner::scanFinished,
            this, [this](int added, int updated, int removed) {
        m_allTracks = m_app->database()->getAllTracks();
        m_libraryPanel->setTracks(m_allTracks);
        statusBar()->showMessage(
            QString("Scan complete: %1 added, %2 updated, %3 removed")
                .arg(added).arg(updated).arg(removed), 5000);
    });

    // Player controls: play track from queue
    connect(m_playerControls, &PlayerControls::seekRequested,
            m_audio, &AudioEngine::seek);
}

void MainWindow::openMusicFolder() {
    QString dir = QFileDialog::getExistingDirectory(
        this, "Select Music Folder", QString(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.isEmpty()) return;

    if (!m_musicDirs.contains(dir)) {
        m_musicDirs.append(dir);
        m_app->settings()->setValue("musicDirectories", m_musicDirs);
    }

    m_scanner->scanDirectory(dir);
}

void MainWindow::playTrack(const Track& track) {
    m_audio->load(track.fileUrl);
    m_audio->play();

    // Update lyrics
    m_lyricsWidget->setLyrics(track.syncedLyrics, track.plainLyrics);

    // Update play count
    Track updated = track;
    updated.playCount++;
    updated.lastPlayed = QDateTime::currentDateTime();
    m_app->database()->updateTrack(updated);

    statusBar()->showMessage(
        QString("Now playing: %1 - %2").arg(track.displayArtist(), track.displayTitle()));
}

void MainWindow::playNext() {
    if (m_currentQueue.isEmpty()) return;

    // Auto-advance logic
    if (m_currentIndex < m_currentQueue.size() - 1) {
        m_currentIndex++;
        playTrack(m_currentQueue[m_currentIndex]);
    }
}

void MainWindow::playPrevious() {
    if (m_currentQueue.isEmpty() || m_currentIndex <= 0) return;
    m_currentIndex--;
    playTrack(m_currentQueue[m_currentIndex]);
}

void MainWindow::showTagEditor(const QList<Track>& tracks) {
    // Placeholder - would open TagEditorDialog
}

void MainWindow::showOnlineSearch() {
    // Placeholder - would open OnlineSearchDialog
}
