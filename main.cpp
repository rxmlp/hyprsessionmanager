#include <QApplication>
#include <QStringList>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QProcess>
#include "SessionManager.h"


QString findLatestCacheFile(const QString &cacheDirPath) {
    QDir cacheDir(cacheDirPath);
    if (!cacheDir.exists()) return QString();

    QStringList filters{"session-*"};
    QFileInfoList files = cacheDir.entryInfoList(filters, QDir::Files, QDir::Time);
    if (files.isEmpty()) return QString();

    return files.first().absoluteFilePath();
}

int main(int argc, char *argv[]) {
    qputenv("QT_QPA_PLATFORM", "wayland");

    QApplication app(argc, argv);
    QStringList args = app.arguments();
    QString cacheDir = QDir::homePath() + "/.cache/hyprsession";

    SessionManager sessionManager;

    if (args.contains("--new-cache")) {
        sessionManager.createNewCache();
        return 0; // Exit after creating cache
    }

    if (args.contains("--restore-latest")) {
        sessionManager.restoreLatestSession();
        return 0; // Exit after restoring latest
    }

    // Default: launch full GUI
    sessionManager.resize(480, 320);
    sessionManager.show();

    return app.exec();
}
