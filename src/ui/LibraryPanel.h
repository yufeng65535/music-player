#pragma once

#include <QWidget>
#include <QList>
#include "model/Track.h"

class Database;
class QTableView;
class QSortFilterProxyModel;
class QStandardItemModel;
class QLineEdit;

class LibraryPanel : public QWidget {
    Q_OBJECT
public:
    explicit LibraryPanel(Database* db, QWidget* parent = nullptr);

    QTableView* trackTable() { return m_tableView; }

    void setTracks(const QList<Track>& tracks);
    void filterTracks(const QString& query);

signals:
    void trackActivated(const Track& track);
    void searchChanged(const QString& query);

private:
    void setupUI();

    Database* m_db = nullptr;
    QTableView* m_tableView = nullptr;
    QStandardItemModel* m_model = nullptr;
    QSortFilterProxyModel* m_proxyModel = nullptr;
    QLineEdit* m_searchEdit = nullptr;

    QList<Track> m_tracks;
};
