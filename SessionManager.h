#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QWidget>

class QListWidget;
class QPushButton;

class SessionManager : public QWidget {
    Q_OBJECT

public:
    explicit SessionManager(QWidget* parent = nullptr);

    // Make these public to allow CLI usage from main.cpp
    void createNewCache();
    void restoreLatestSession();

private slots:
    void refreshSessionList();
    void restoreSelectedSession();
    void removeSelectedSession();

private:
    bool restoreSession(const QString &filepath);

    QString cacheDir;
    QListWidget *listWidget;
    QPushButton *restoreLatestButton;
    QPushButton *restoreButton;
    QPushButton *removeButton;
    QPushButton *newCacheButton;
};

#endif // SESSIONMANAGER_H
