#pragma once

#include <QString>
#include <QByteArray>
#include "model/Track.h"

class TagManager {
public:
    // Read all tags from a file, populate a Track
    static Track readTags(const QString& filePath);

    // Write track metadata back to file
    static bool writeTags(const QString& filePath, const Track& track);

    // Supported file extensions
    static bool isSupported(const QString& filePath);
    static QStringList supportedExtensions();
};
