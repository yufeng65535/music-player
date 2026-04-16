#include "ui/TagEditorDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QGroupBox>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include "core/TagManager.h"

TagEditorDialog::TagEditorDialog(const QList<Track>& tracks, QWidget* parent)
    : QDialog(parent), m_tracks(tracks)
{
    setWindowTitle(QString("Edit Tags (%1 tracks)").arg(tracks.size()));
    resize(500, 400);
    setupUI();

    // Show first track's tags as defaults
    if (!tracks.isEmpty()) {
        const Track& first = tracks.first();
        m_titleEdit->setText(first.title);
        m_artistEdit->setText(first.artist);
        m_albumEdit->setText(first.album);
        m_genreEdit->setText(first.genre);
        if (first.year) m_yearEdit->setText(QString::number(*first.year));
    }
}

void TagEditorDialog::setupUI() {
    auto* layout = new QVBoxLayout(this);

    // File list
    layout->addWidget(new QLabel("Selected files:", this));
    m_fileList = new QListWidget(this);
    for (const auto& t : m_tracks) {
        m_fileList->addItem(t.filePath);
    }
    layout->addWidget(m_fileList);

    // Tag fields
    auto* fields = new QGroupBox("Tags", this);
    auto* form = new QFormLayout(fields);

    m_titleEdit = new QLineEdit(this);
    form->addRow("Title:", m_titleEdit);

    m_artistEdit = new QLineEdit(this);
    form->addRow("Artist:", m_artistEdit);

    m_albumEdit = new QLineEdit(this);
    form->addRow("Album:", m_albumEdit);

    m_genreEdit = new QLineEdit(this);
    form->addRow("Genre:", m_genreEdit);

    m_yearEdit = new QLineEdit(this);
    m_yearEdit->setValidator(new QRegularExpressionValidator(QRegularExpression(R"(\d{0,4})"), this));
    form->addRow("Year:", m_yearEdit);

    layout->addWidget(fields);

    // Buttons
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    auto* saveBtn = new QPushButton("Save", this);
    connect(saveBtn, &QPushButton::clicked, this, &TagEditorDialog::saveTags);
    btnLayout->addWidget(saveBtn);

    auto* cancelBtn = new QPushButton("Cancel", this);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);

    layout->addLayout(btnLayout);
}

void TagEditorDialog::saveTags() {
    QString title = m_titleEdit->text();
    QString artist = m_artistEdit->text();
    QString album = m_albumEdit->text();
    QString genre = m_genreEdit->text();
    QString yearStr = m_yearEdit->text();

    int successCount = 0;
    for (auto& track : m_tracks) {
        Track updated = track;

        // Apply edits (only if non-empty, or clear if explicitly empty)
        if (m_titleEdit->isModified() || m_tracks.size() == 1)
            updated.title = title;
        if (m_artistEdit->isModified() || m_tracks.size() == 1)
            updated.artist = artist;
        if (m_albumEdit->isModified() || m_tracks.size() == 1)
            updated.album = album;
        if (m_genreEdit->isModified() || m_tracks.size() == 1)
            updated.genre = genre;
        if (m_yearEdit->isModified() || m_tracks.size() == 1) {
            if (!yearStr.isEmpty())
                updated.year = yearStr.toUInt();
            else
                updated.year = std::nullopt;
        }

        if (TagManager::writeTags(updated.filePath, updated)) {
            successCount++;
        }
    }

    QMessageBox::information(this, "Save Complete",
        QString("Successfully updated %1 of %2 files.").arg(successCount).arg(m_tracks.size()));
    accept();
}
