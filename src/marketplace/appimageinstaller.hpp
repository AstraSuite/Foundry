#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QList>
#include <QProcess>
#include <QQmlEngine>
#include <QJSEngine>
#include <qqmlintegration.h>

class AppImageInstaller : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool isInstalling READ isInstalling NOTIFY isInstallingChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit AppImageInstaller(QObject* parent = nullptr);

    static AppImageInstaller* create(QQmlEngine*, QJSEngine*);

    bool isInstalling() const { return m_isInstalling; }
    QString statusMessage() const { return m_statusMessage; }

    Q_INVOKABLE bool installAppImage(const QString& fileUrlOrPath);
    Q_INVOKABLE QVariantList listInstalledAppImages();
    Q_INVOKABLE bool uninstallAppImage(const QString& appIdentifier);
    Q_INVOKABLE bool launchAppImage(const QString& appIdentifier);

    static QVariantList installedAppImages();
    static QVariantMap findInstalledAppImage(const QString& appIdentifier);

signals:
    void isInstallingChanged();
    void statusMessageChanged();
    void appImageInstalled(const QString& appName, const QString& desktopPath);
    void appImageInstallationFailed(const QString& reason);

private:
    void setStatus(bool installing, const QString& message);
    QString extractIcon(const QString& tempExtractDir, const QString& appName);
    QString generateDesktopFile(const QString& appName, const QString& execPath, const QString& iconPath, const QString& displayName);

    bool m_isInstalling{false};
    QString m_statusMessage;
};
