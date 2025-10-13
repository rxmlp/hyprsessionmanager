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

int askAndRestoreLatestSession(const QString &cacheDir) {
    QString latestCacheFile = findLatestCacheFile(cacheDir);

    if (latestCacheFile.isEmpty()) {
        QMessageBox::critical(nullptr, "No Cached Sessions",
                              "No cached session files found in:\n" + cacheDir);
        return 1;
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle("Restore Session");
    msgBox.setText("Do you want to restore your latest session?\n\n" + latestCacheFile);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);

    int ret = msgBox.exec();

    if (ret == QMessageBox::Yes) {
        QFile file(latestCacheFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(nullptr, "Failed to open cache file",
                                  "Failed to open cache file:\n" + latestCacheFile);
            return 1;
        }
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString desktopfile = in.readLine().trimmed();
            if (!desktopfile.isEmpty()) {
                QString filename = QFileInfo(desktopfile).fileName();
                QProcess::startDetached("gtk-launch", QStringList() << filename);
            }
        }
        file.close();
        return 0;
    } else {
        return 1;
    }
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

    if (args.contains("--ask-restore-latest")) {
        return askAndRestoreLatestSession(cacheDir);
    }

    // Default: launch full GUI
    sessionManager.resize(480, 320);
    sessionManager.show();

    return app.exec();
}
