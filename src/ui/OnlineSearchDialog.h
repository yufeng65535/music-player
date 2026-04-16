#pragma once

#include <QDialog>
#include <QList>
#include "network/JamendoService.h"
#include "network/LyricsService.h"

class QTabWidget;
class QTableView;
class QLineEdit;
class QPushButton;
class QProgressBar;
class QStandardItemModel;
class QTextEdit;

class OnlineSearchDialog : public QDialog {
    Q_OBJECT
public:
    explicit OnlineSearchDialog(const QString& jamendoClientId, QWidget* parent = nullptr);

signals:
    void trackDownloaded(const QString& filePath);

private:
    void setupMusicTab();
    void setupLyricsTab();
    void searchMusic();
    void searchLyrics();
    void downloadSelectedTrack();
    void onJamendoResults(const QList<JamendoTrack>& tracks);

    QString m_jamendoClientId;
    QTabWidget* m_tabs = nullptr;

    // Music tab
    QTableView* m_musicTable = nullptr;
    QStandardItemModel* m_musicModel = nullptr;
    QLineEdit* m_musicSearchEdit = nullptr;
    QPushButton* m_musicSearchBtn = nullptr;
    QProgressBar* m_downloadProgress = nullptr;

    // Lyrics tab
    QTextEdit* m_lyricsEdit = nullptr;
    QLineEdit* m_lyricsSearchEdit = nullptr;
    QPushButton* m_lyricsSearchBtn = nullptr;

    QList<JamendoTrack> m_jamendoResults;
    JamendoService* m_jamendo = nullptr;
    LyricsService* m_lyricsSvc = nullptr;
};
