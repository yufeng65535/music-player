#pragma once

#include <QDialog>
#include "model/Track.h"

class QLineEdit;
class QLabel;
class QPushButton;
class QListWidget;

class TagEditorDialog : public QDialog {
    Q_OBJECT
public:
    explicit TagEditorDialog(const QList<Track>& tracks, QWidget* parent = nullptr);

private:
    void setupUI();
    void saveTags();

    QList<Track> m_tracks;

    QLineEdit* m_titleEdit = nullptr;
    QLineEdit* m_artistEdit = nullptr;
    QLineEdit* m_albumEdit = nullptr;
    QLineEdit* m_genreEdit = nullptr;
    QLineEdit* m_yearEdit = nullptr;
    QListWidget* m_fileList = nullptr;
};
