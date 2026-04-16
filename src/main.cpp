#include <QApplication>
#include <QFile>
#include <QFont>
#include <QDebug>
#include "app/Application.h"
#include "ui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("MusicPlayer");
    app.setOrganizationName("MusicPlayer");
    app.setApplicationVersion("0.1.0");

    // Load dark theme
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }

    // Default font
    QFont font = app.font();
    font.setPointSize(10);
    app.setFont(font);

    // Initialize application backend
    Application* backend = new Application(&app);
    if (!backend->init()) {
        qCritical() << "Application initialization failed";
        return 1;
    }

    // Register metatypes
    qRegisterMetaType<Track>();
    qRegisterMetaType<QList<Track>>();
    qRegisterMetaType<Playlist>();
    qRegisterMetaType<QList<Playlist>>();

    MainWindow window(backend);
    window.resize(1200, 750);
    window.show();

    return app.exec();
}
