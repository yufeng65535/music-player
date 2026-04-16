#include "app/Application.h"

#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QCoreApplication>

Application::Application(QObject* parent)
    : QObject(parent)
{
}

bool Application::init() {
    // Data directory: ~/.local/share/musicplayer/
    m_dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    ensureDataDirs();

    // Database
    QString dbPath = m_dataDir + "/music.db";
    m_db = new Database(dbPath, this);
    if (!m_db->init()) {
        qCritical() << "Failed to initialize database at" << dbPath;
        return false;
    }

    // Settings
    m_settings = new QSettings("MusicPlayer", "MusicPlayer", this);

    qInfo() << "Application initialized. Data dir:" << m_dataDir;
    return true;
}

void Application::ensureDataDirs() {
    QDir().mkpath(m_dataDir);
    QDir().mkpath(m_dataDir + "/covers");
    QDir().mkpath(m_dataDir + "/lyrics");
    QDir().mkpath(m_dataDir + "/downloads");
}
