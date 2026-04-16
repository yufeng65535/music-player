#include "ui/PlaylistPanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>
#include "db/Database.h"

PlaylistPanel::PlaylistPanel(Database* db, QWidget* parent)
    : QWidget(parent), m_db(db)
{
    setupUI();
    refreshPlaylists();
}

void PlaylistPanel::setupUI() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Header
    auto* header = new QHBoxLayout();
    header->addWidget(new QLabel("Playlists", this));
    header->addStretch();

    auto* addBtn = new QPushButton("+", this);
    addBtn->setFixedSize(24, 24);
    addBtn->setToolTip("New Playlist");
    connect(addBtn, &QPushButton::clicked, this, &PlaylistPanel::createNewPlaylist);
    header->addWidget(addBtn);
    layout->addLayout(header);

    // Playlist list
    m_listWidget = new QListWidget(this);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(m_listWidget);

    // Buttons
    auto* btnLayout = new QHBoxLayout();

    auto* loadBtn = new QPushButton("Load", this);
    connect(loadBtn, &QPushButton::clicked, this, &PlaylistPanel::loadSelectedPlaylist);
    btnLayout->addWidget(loadBtn);

    auto* delBtn = new QPushButton("Delete", this);
    connect(delBtn, &QPushButton::clicked, this, &PlaylistPanel::deleteSelectedPlaylist);
    btnLayout->addWidget(delBtn);

    layout->addLayout(btnLayout);

    // Double-click to load
    connect(m_listWidget, &QListWidget::itemDoubleClicked,
            this, &PlaylistPanel::loadSelectedPlaylist);
}

void PlaylistPanel::refreshPlaylists() {
    m_playlists = m_db->getAllPlaylists();
    m_listWidget->clear();

    for (const auto& p : m_playlists) {
        auto* item = new QListWidgetItem(
            QString("%1 (%2 tracks)").arg(p.name).arg(p.trackCount()));
        item->setData(Qt::UserRole, QVariant::fromValue<qlonglong>(p.id));
        m_listWidget->addItem(item);
    }
}

void PlaylistPanel::createNewPlaylist() {
    QString name = QInputDialog::getText(this, "New Playlist", "Playlist name:");
    if (name.isEmpty()) return;

    int64_t id = m_db->createPlaylist(name);
    if (id > 0) {
        refreshPlaylists();
    }
}

void PlaylistPanel::deleteSelectedPlaylist() {
    auto* item = m_listWidget->currentItem();
    if (!item) return;

    int64_t id = item->data(Qt::UserRole).toLongLong();

    auto result = QMessageBox::question(this, "Delete Playlist",
        "Delete playlist \"" + item->text() + "\"?",
        QMessageBox::Yes | QMessageBox::No);
    if (result != QMessageBox::Yes) return;

    m_db->deletePlaylist(id);
    refreshPlaylists();
}

void PlaylistPanel::loadSelectedPlaylist() {
    auto* item = m_listWidget->currentItem();
    if (!item) return;

    int64_t id = item->data(Qt::UserRole).toLongLong();
    emit playlistSelected(id);
}
