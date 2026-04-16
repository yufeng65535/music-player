#include "ui/OnlineSearchDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTableView>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QStandardItemModel>
#include <QTextEdit>
#include <QLabel>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include "core/LyricsManager.h"
#include "network/JamendoService.h"
#include "network/LyricsService.h"

OnlineSearchDialog::OnlineSearchDialog(const QString& jamendoClientId, QWidget* parent)
    : QDialog(parent), m_jamendoClientId(jamendoClientId)
{
    setWindowTitle("Online Search");
    resize(700, 500);

    m_jamendo = new JamendoService(jamendoClientId, this);
    m_lyricsSvc = new LyricsService(this);

    auto* layout = new QVBoxLayout(this);
    m_tabs = new QTabWidget(this);
    layout->addWidget(m_tabs);

    setupMusicTab();
    setupLyricsTab();

    // Jamendo signals
    connect(m_jamendo, &JamendoService::searchResults,
            this, &OnlineSearchDialog::onJamendoResults);
    connect(m_jamendo, &JamendoService::downloadFinished,
            this, [this](const QString& path) {
        emit trackDownloaded(path);
        QMessageBox::information(this, "Download Complete",
            "Track saved to:\n" + path);
    });
    connect(m_jamendo, &JamendoService::errorOccurred,
            this, [this](const QString& msg) {
        QMessageBox::warning(this, "Download Error", msg);
    });

    // Lyrics signals
    connect(m_lyricsSvc, &LyricsService::lyricsFetched,
            this, [this](const QString& plain, const QString& synced) {
        m_lyricsEdit->setPlainText(
            synced.isEmpty() ? plain : LyricsManager::lrcToPlain(synced));
    });
}

void OnlineSearchDialog::setupMusicTab() {
    auto* musicTab = new QWidget();
    auto* layout = new QVBoxLayout(musicTab);

    // Search bar
    auto* searchLayout = new QHBoxLayout();
    m_musicSearchEdit = new QLineEdit(musicTab);
    m_musicSearchEdit->setPlaceholderText("Search free music on Jamendo...");
    m_musicSearchBtn = new QPushButton("Search", musicTab);
    connect(m_musicSearchBtn, &QPushButton::clicked, this, &OnlineSearchDialog::searchMusic);
    connect(m_musicSearchEdit, &QLineEdit::returnPressed, this, &OnlineSearchDialog::searchMusic);
    searchLayout->addWidget(m_musicSearchEdit);
    searchLayout->addWidget(m_musicSearchBtn);
    layout->addLayout(searchLayout);

    // Results table
    m_musicModel = new QStandardItemModel(this);
    m_musicModel->setHorizontalHeaderLabels({
        "Title", "Artist", "Album", "Duration", "Tags"
    });

    m_musicTable = new QTableView(musicTab);
    m_musicTable->setModel(m_musicModel);
    m_musicTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_musicTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_musicTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_musicTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    layout->addWidget(m_musicTable);

    // Download progress
    m_downloadProgress = new QProgressBar(musicTab);
    m_downloadProgress->hide();
    layout->addWidget(m_downloadProgress);

    // Download button
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    auto* dlBtn = new QPushButton("Download Selected", musicTab);
    connect(dlBtn, &QPushButton::clicked, this, &OnlineSearchDialog::downloadSelectedTrack);
    btnLayout->addWidget(dlBtn);
    layout->addLayout(btnLayout);

    m_tabs->addTab(musicTab, "Music");
}

void OnlineSearchDialog::setupLyricsTab() {
    auto* lyricsTab = new QWidget();
    auto* layout = new QVBoxLayout(lyricsTab);

    // Search bar
    auto* searchLayout = new QHBoxLayout();
    m_lyricsSearchEdit = new QLineEdit(lyricsTab);
    m_lyricsSearchEdit->setPlaceholderText("Search lyrics (artist - title)...");
    m_lyricsSearchBtn = new QPushButton("Search", lyricsTab);
    connect(m_lyricsSearchBtn, &QPushButton::clicked, this, &OnlineSearchDialog::searchLyrics);
    connect(m_lyricsSearchEdit, &QLineEdit::returnPressed, this, &OnlineSearchDialog::searchLyrics);
    searchLayout->addWidget(m_lyricsSearchEdit);
    searchLayout->addWidget(m_lyricsSearchBtn);
    layout->addLayout(searchLayout);

    // Lyrics display
    m_lyricsEdit = new QTextEdit(lyricsTab);
    m_lyricsEdit->setReadOnly(true);
    m_lyricsEdit->setPlaceholderText("Lyrics will appear here...");
    layout->addWidget(m_lyricsEdit);

    m_tabs->addTab(lyricsTab, "Lyrics");
}

void OnlineSearchDialog::searchMusic() {
    QString query = m_musicSearchEdit->text().trimmed();
    if (query.isEmpty()) return;
    m_jamendo->search(query);
}

void OnlineSearchDialog::searchLyrics() {
    QString query = m_lyricsSearchEdit->text().trimmed();
    if (query.isEmpty()) return;
    m_lyricsSvc->searchLyrics(query);
}

void OnlineSearchDialog::downloadSelectedTrack() {
    int row = m_musicTable->currentIndex().row();
    if (row < 0 || row >= m_jamendoResults.size()) {
        QMessageBox::warning(this, "No Selection", "Please select a track to download.");
        return;
    }

    const JamendoTrack& track = m_jamendoResults[row];
    QString destDir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation)
                      + "/MusicPlayer/Downloads";

    m_downloadProgress->show();
    m_downloadProgress->setValue(0);

    connect(m_jamendo, &JamendoService::downloadProgress, this,
            [this](const QString&, float progress) {
        m_downloadProgress->setValue(static_cast<int>(progress * 100));
    }, Qt::SingleShotConnection);

    m_jamendo->downloadTrack(track, destDir);
}

void OnlineSearchDialog::onJamendoResults(const QList<JamendoTrack>& tracks) {
    m_jamendoResults = tracks;
    m_musicModel->setRowCount(0);

    for (const auto& t : tracks) {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(t.title));
        row.append(new QStandardItem(t.artist));
        row.append(new QStandardItem(t.album));
        int min = t.durationSec / 60;
        int sec = t.durationSec % 60;
        row.append(new QStandardItem(QString("%1:%2").arg(min).arg(sec, 2, 10, QChar('0'))));
        row.append(new QStandardItem(t.tags.join(", ")));
        m_musicModel->appendRow(row);
    }
}
