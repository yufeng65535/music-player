#pragma once

#include <QObject>
#include <QSettings>
#include "db/Database.h"

class Application : public QObject {
    Q_OBJECT
public:
    explicit Application(QObject* parent = nullptr);

    bool init();

    Database* database() { return m_db; }
    QSettings* settings() { return m_settings; }

    QString dataDir() const { return m_dataDir; }

private:
    void ensureDataDirs();

    Database* m_db = nullptr;
    QSettings* m_settings = nullptr;
    QString m_dataDir;
};
