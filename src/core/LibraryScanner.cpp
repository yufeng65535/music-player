#include "core/LibraryScanner.h"
#include "core/TagManager.h"
#include "db/Database.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QDateTime>
#include <QtConcurrent>
#include <QDebug>

LibraryScanner::LibraryScanner(Database* db, QObject* parent)
    : QObject(parent), m_db(db)
{
}

QStringList LibraryScanner::findAudioFiles(const QString& path) {
    QStringList files;
    auto exts = TagManager::supportedExtensions();
    QDirIterator it(path, exts,
                   QDir::Files | QDir::Readable | QDir::NoDotAndDotDot,
                   QDirIterator::Subdirectories);
    while (it.hasNext()) {
        files.append(it.next());
    }
    return files;
}

QDateTime LibraryScanner::getFileMtime(const QString& path) {
    return QFileInfo(path).lastModified();
}

void LibraryScanner::scanDirectory(const QString& path) {
    scanDirectories(QStringList{path});
}

void LibraryScanner::scanDirectories(const QStringList& paths) {
    emit scanStarted();

    // Collect all audio files
    QStringList allFiles;
    for (const auto& path : paths) {
        allFiles.append(findAudioFiles(path));
    }

    int total = allFiles.size();
    int added = 0;
    int updated = 0;
    int skipped = 0;

    for (int i = 0; i < total && !m_cancelled; ++i) {
        const QString& file = allFiles[i];

        // Check if already in DB with same mtime
        if (m_db->trackExists(file)) {
            auto existing = m_db->searchTracks(QFileInfo(file).fileName());
            // Simple check: if file exists, skip for now (incremental would check mtime)
            skipped++;
        } else {
            Track track = TagManager::readTags(file);
            if (m_db->insertTrack(track)) {
                added++;
            }
        }

        if (i % 50 == 0 || i == total - 1) {
            emit scanProgress(i + 1, total);
        }
    }

    emit scanFinished(added, updated, 0);
}

void LibraryScanner::rescanAll() {
    // Get all stored music directories from settings
    // For now, this is a placeholder - the MainWindow will manage directories
    emit scanStarted();
    emit scanFinished(0, 0, 0);
}
