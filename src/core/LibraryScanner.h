#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>

class Database;

class LibraryScanner : public QObject {
    Q_OBJECT
public:
    explicit LibraryScanner(Database* db, QObject* parent = nullptr);

    // Scan a single directory (recursive)
    void scanDirectory(const QString& path);

    // Scan multiple directories
    void scanDirectories(const QStringList& paths);

    // Incremental rescan: only check files with changed mtime
    void rescanAll();

signals:
    void scanStarted();
    void scanProgress(int filesScanned, int totalFiles);
    void scanFinished(int tracksAdded, int tracksUpdated, int tracksRemoved);
    void errorOccurred(const QString& message);

private:
    QStringList findAudioFiles(const QString& path);
    QDateTime getFileMtime(const QString& path);

    Database* m_db = nullptr;
    bool m_cancelled = false;
};
