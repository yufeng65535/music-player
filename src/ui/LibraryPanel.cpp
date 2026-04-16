#include "ui/LibraryPanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QLineEdit>
#include <QLabel>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QDesktopServices>
#include <QUrl>

#include "db/Database.h"

LibraryPanel::LibraryPanel(Database* db, QWidget* parent)
    : QWidget(parent), m_db(db)
{
    setupUI();
}

void LibraryPanel::setupUI() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Search bar
    auto* searchLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search library...");
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, [this](const QString& text) {
        if (m_proxyModel) {
            m_proxyModel->setFilterFixedString(text);
        }
        emit searchChanged(text);
    });
    searchLayout->addWidget(m_searchEdit);
    layout->addLayout(searchLayout);

    // Track table
    m_tableView = new QTableView(this);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setSortingEnabled(true);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->verticalHeader()->hide();
    m_tableView->setShowGrid(false);

    // Model
    m_model = new QStandardItemModel(this);
    m_model->setHorizontalHeaderLabels({
        "Title", "Artist", "Album", "Duration", "Genre", "Year"
    });

    // Proxy model for filtering
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(-1);  // Search all columns
    m_tableView->setModel(m_proxyModel);

    // Column sizing
    m_tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    m_tableView->setColumnWidth(3, 70);
    m_tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_tableView->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
    m_tableView->setColumnWidth(5, 50);

    layout->addWidget(m_tableView);

    // Double-click to play
    connect(m_tableView, &QTableView::doubleClicked, this, [this](const QModelIndex& index) {
        int sourceRow = m_proxyModel->mapToSource(index).row();
        if (sourceRow >= 0 && sourceRow < m_tracks.size()) {
            emit trackActivated(m_tracks[sourceRow]);
        }
    });

    // Context menu
    m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tableView, &QTableView::customContextMenuRequested,
            this, [this](const QPoint& pos) {
        QModelIndex idx = m_tableView->indexAt(pos);
        if (!idx.isValid()) return;

        QMenu menu(this);
        menu.addAction("Play", [this, idx]() {
            int sourceRow = m_proxyModel->mapToSource(idx).row();
            if (sourceRow >= 0 && sourceRow < m_tracks.size()) {
                emit trackActivated(m_tracks[sourceRow]);
            }
        });
        menu.addAction("Open File Location", [this, idx]() {
            int sourceRow = m_proxyModel->mapToSource(idx).row();
            if (sourceRow >= 0 && sourceRow < m_tracks.size()) {
                QDesktopServices::openUrl(
                    QUrl::fromLocalFile(m_tracks[sourceRow].filePath));
            }
        });
        menu.addSeparator();
        menu.addAction("Edit Tags");
        menu.addAction("Favorite");
        menu.exec(m_tableView->viewport()->mapToGlobal(pos));
    });
}

void LibraryPanel::setTracks(const QList<Track>& tracks) {
    m_tracks = tracks;
    m_model->setRowCount(0);

    for (const auto& t : tracks) {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(t.displayTitle()));
        row.append(new QStandardItem(t.displayArtist()));
        row.append(new QStandardItem(t.displayAlbum()));
        row.append(new QStandardItem(t.formattedDuration()));
        row.append(new QStandardItem(t.genre));
        row.append(new QStandardItem(t.year ? QString::number(*t.year) : ""));
        m_model->appendRow(row);
    }
}

void LibraryPanel::filterTracks(const QString& query) {
    if (m_proxyModel) {
        m_proxyModel->setFilterFixedString(query);
    }
}
