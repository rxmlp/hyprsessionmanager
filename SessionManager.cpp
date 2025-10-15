#include "SessionManager.h"
#include <QListWidget>
#include <QPushButton>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDateTime>

SessionManager::SessionManager(QWidget *parent)
    : QWidget(parent), cacheDir(QDir::homePath() + "/.cache/hyprsession") {
    listWidget = new QListWidget(this);
    listWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    restoreLatestButton = new QPushButton("Restore Latest", this);
    restoreButton = new QPushButton("Restore Selected", this);
    removeButton = new QPushButton("Remove Selected", this);
    newCacheButton = new QPushButton("New Cache", this);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(restoreLatestButton);
    buttonLayout->addWidget(restoreButton);
    buttonLayout->addWidget(removeButton);
    buttonLayout->addWidget(newCacheButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(listWidget);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    refreshSessionList();

    connect(restoreLatestButton, &QPushButton::clicked, this, &SessionManager::restoreLatestSession);
    connect(restoreButton, &QPushButton::clicked, this, &SessionManager::restoreSelectedSession);
    connect(removeButton, &QPushButton::clicked, this, &SessionManager::removeSelectedSession);
    connect(newCacheButton, &QPushButton::clicked, this, &SessionManager::createNewCache);
}

void SessionManager::refreshSessionList() {
    listWidget->clear();
    QDir dir(cacheDir);
    if (!dir.exists())
        dir.mkpath(".");

    QStringList filters{"session-*"};
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time);

    // Remove sessions beyond the 5 newest
    const int maxSessions = 5;
    if (files.size() > maxSessions) {
        for (int i = maxSessions; i < files.size(); ++i) {
            QFile f(files[i].absoluteFilePath());
            if (!f.remove()) {
                qWarning() << "Failed to remove session file:" << files[i].fileName();
            }
        }
        files = dir.entryInfoList(filters, QDir::Files, QDir::Time);
    }

    // Show up to 5 newest sessions
    for (const QFileInfo &fileInfo : files) {
        QString filename = fileInfo.fileName();
        QString timestampStr = filename;
        timestampStr.remove("session-");

        QDateTime dt = QDateTime::fromString(timestampStr, "yyyyMMdd-HHmmss");
        QString displayName = dt.isValid() ? dt.toString("d MMM yyyy [HH:mm]") : filename;

        QListWidgetItem *item = new QListWidgetItem(displayName, listWidget);
        item->setData(Qt::UserRole, filename);
    }

    if (listWidget->count() > 0)
        listWidget->setCurrentRow(0);
}


void SessionManager::restoreLatestSession() {
    QDir dir(cacheDir);
    QStringList filters{"session-*"};
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time);

    if (files.isEmpty()) {
        QMessageBox::information(this, "No Sessions", "No cached sessions found to restore.");
        return;
    }

    QFileInfo latestSession = files.first(); // sorted by modification time descending

    QString filename = latestSession.fileName();
    QString filepath = cacheDir + "/" + filename;

    QMessageBox::StandardButton ret = QMessageBox::question(this, "Restore Latest Session",
        "Restore latest session:\n" + latestSession.fileName() + "?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (ret == QMessageBox::Yes) {
        if (!restoreSession(filepath)) {
            QMessageBox::warning(this, "Restore Failed", "Failed to restore session:\n" + filename);
        }
    }
}


void SessionManager::restoreSelectedSession() {
    QListWidgetItem *item = listWidget->currentItem();
    if (!item) {
        QMessageBox::information(this, "No Selection", "Please select a session to restore.");
        return;
    }

    QString filename = item->data(Qt::UserRole).toString();
    QString filepath = cacheDir + "/" + filename;

    if (!restoreSession(filepath)) {
        QMessageBox::warning(this, "Restore Failed", "Failed to restore session:\n" + item->text());
    }
}

void SessionManager::removeSelectedSession() {
    QListWidgetItem *item = listWidget->currentItem();
    if (!item) {
        QMessageBox::information(this, "No Selection", "Please select a session to remove.");
        return;
    }

    int ret = QMessageBox::question(this, "Confirm Remove",
        "Are you sure you want to remove the selected session?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (ret != QMessageBox::Yes)
        return;

    QString filename = item->data(Qt::UserRole).toString();
    QString filepath = cacheDir + "/" + filename;

    QFile f(filepath);
    if (!f.remove()) {
        QMessageBox::warning(this, "Remove Failed", "Failed to remove session:\n" + item->text());
    }
    refreshSessionList();
}

void SessionManager::createNewCache() {
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    QString cacheFile = cacheDir + "/session-" + timestamp;

QString script = QString(R"(
    mkdir -p '%1'
    hyprctl clients -j | jq -r '.[].initialClass' | sort -u | while read -r class; do
        if [ -z "$class" ]; then continue; fi
        if [ "$class" = "hyprsessionmanager" ]; then continue; fi
        desktop_path="$HOME/.local/share/applications/${class}.desktop"
        if [ ! -f "$desktop_path" ]; then
            desktop_path="/usr/share/applications/${class}.desktop"
        fi
        if [ -f "$desktop_path" ]; then
            echo "$desktop_path"
        fi
    done > '%2'
)").arg(cacheDir, cacheFile);

    QProcess process;
    process.start("bash", QStringList() << "-c" << script);
    if (!process.waitForFinished(5000)) {
        QMessageBox::warning(this, "Cache Creation Failed",
                             "Timeout while creating new cache session.");
        return;
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        QMessageBox::warning(this, "Cache Creation Failed",
                             "Failed to create new cache session:\n" + process.readAllStandardError());
        return;
    }

    refreshSessionList();
    QMessageBox::information(this, "Cache Created", "New session cached as:\n" + cacheFile);
}

bool SessionManager::restoreSession(const QString &filepath) {
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString desktopfile = in.readLine().trimmed();
        if (!desktopfile.isEmpty()) {
            QString filename = QFileInfo(desktopfile).fileName();
            QProcess::startDetached("gtk-launch", QStringList() << filename);
        }
    }
    file.close();
    return true;
}

#include "SessionManager.moc"
