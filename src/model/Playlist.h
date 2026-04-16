#pragma once

#include <QString>
#include <QList>
#include <QDateTime>
#include <cstdint>

struct Playlist {
    int64_t id = 0;
    QString name;
    QString description;
    QDateTime createdAt;
    QDateTime modifiedAt;

    // Track IDs in user-defined order
    QList<int64_t> trackIds;

    bool isSmart = false;
    QString smartRuleJson;

    int trackCount() const { return trackIds.size(); }
};

Q_DECLARE_METATYPE(Playlist)
Q_DECLARE_METATYPE(QList<Playlist>)
