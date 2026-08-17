#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QtQmlIntegration/qqmlintegration.h>
#include <QtQml/qqmlengine.h>

class PackageManager;

class TrayManager : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool trayEnabled READ trayEnabled WRITE setTrayEnabled NOTIFY trayEnabledChanged)
    Q_PROPERTY(bool closeToTray READ closeToTray WRITE setCloseToTray NOTIFY closeToTrayChanged)
    Q_PROPERTY(bool autostart READ autostart WRITE setAutostart NOTIFY autostartChanged)
    Q_PROPERTY(int checkIntervalHours READ checkIntervalHours WRITE setCheckIntervalHours NOTIFY checkIntervalHoursChanged)
    Q_PROPERTY(int notifyThreshold READ notifyThreshold WRITE setNotifyThreshold NOTIFY notifyThresholdChanged)
    Q_PROPERTY(bool autoUpdateFlatpak READ autoUpdateFlatpak WRITE setAutoUpdateFlatpak NOTIFY autoUpdateFlatpakChanged)
    Q_PROPERTY(bool autoUpdatePacman READ autoUpdatePacman WRITE setAutoUpdatePacman NOTIFY autoUpdatePacmanChanged)
    Q_PROPERTY(bool autoUpdateAur READ autoUpdateAur WRITE setAutoUpdateAur NOTIFY autoUpdateAurChanged)
    Q_PROPERTY(bool useCaelestiaUpdate READ useCaelestiaUpdate WRITE setUseCaelestiaUpdate NOTIFY useCaelestiaUpdateChanged)
    Q_PROPERTY(bool autoUpdateCaelestia READ autoUpdateCaelestia WRITE setAutoUpdateCaelestia NOTIFY autoUpdateCaelestiaChanged)
    Q_PROPERTY(int pendingUpdateCount READ pendingUpdateCount NOTIFY pendingUpdateCountChanged)
    Q_PROPERTY(bool isChecking READ isChecking NOTIFY isCheckingChanged)
    Q_PROPERTY(QString lastCheckString READ lastCheckString NOTIFY lastCheckStringChanged)

public:
    static TrayManager* instance();
    static TrayManager* create(QQmlEngine* = nullptr, QJSEngine* = nullptr);

    explicit TrayManager(QObject* parent = nullptr);
    ~TrayManager() override;

    void setPackageManager(PackageManager* pm);

    [[nodiscard]] bool trayEnabled() const { return m_trayEnabled; }
    [[nodiscard]] bool closeToTray() const { return m_closeToTray; }
    [[nodiscard]] bool autostart() const { return m_autostart; }
    [[nodiscard]] int checkIntervalHours() const { return m_checkIntervalHours; }
    [[nodiscard]] int notifyThreshold() const { return m_notifyThreshold; }
    [[nodiscard]] bool autoUpdateFlatpak() const { return m_autoUpdateFlatpak; }
    [[nodiscard]] bool autoUpdatePacman() const { return m_autoUpdatePacman; }
    [[nodiscard]] bool autoUpdateAur() const { return m_autoUpdateAur; }
    [[nodiscard]] bool useCaelestiaUpdate() const { return m_useCaelestiaUpdate; }
    [[nodiscard]] bool autoUpdateCaelestia() const { return m_autoUpdateCaelestia; }
    [[nodiscard]] int pendingUpdateCount() const { return m_pendingUpdateCount; }
    [[nodiscard]] bool isChecking() const { return m_isChecking; }
    [[nodiscard]] QString lastCheckString() const;

    void setTrayEnabled(bool enabled);
    void setCloseToTray(bool enabled);
    void setAutostart(bool enabled);
    void setCheckIntervalHours(int hours);
    void setNotifyThreshold(int count);
    void setAutoUpdateFlatpak(bool enabled);
    void setAutoUpdatePacman(bool enabled);
    void setAutoUpdateAur(bool enabled);
    void setUseCaelestiaUpdate(bool enabled);
    void setAutoUpdateCaelestia(bool enabled);

    Q_INVOKABLE void checkForUpdates();
    Q_INVOKABLE void updateAll();
    Q_INVOKABLE void showMainWindow();
    Q_INVOKABLE void toggleMainWindow();
    Q_INVOKABLE void showNotification(const QString& title, const QString& message);

signals:
    void trayEnabledChanged();
    void closeToTrayChanged();
    void autostartChanged();
    void checkIntervalHoursChanged();
    void notifyThresholdChanged();
    void autoUpdateFlatpakChanged();
    void autoUpdatePacmanChanged();
    void autoUpdateAurChanged();
    void useCaelestiaUpdateChanged();
    void autoUpdateCaelestiaChanged();
    void pendingUpdateCountChanged();
    void isCheckingChanged();
    void lastCheckStringChanged();

    void requestShowMainWindow();
    void requestToggleMainWindow();
    void requestShowUpdatesPage();
    void requestShowSettingsPage();

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onScheduledCheckTimeout();
    void onUpdatesRefreshed();

private:
    void initTray();
    void updateTrayIcon();
    void updateTrayMenu();
    void updateTrayIconState();
    void restartTimer();
    void updateAutostartFile(bool enable);
    bool checkAutostartFileExists() const;

    QSystemTrayIcon* m_trayIcon = nullptr;
    QMenu* m_trayMenu = nullptr;
    QAction* m_statusAction = nullptr;
    QAction* m_checkAction = nullptr;
    QAction* m_updateAllAction = nullptr;
    QAction* m_openAction = nullptr;
    QAction* m_settingsAction = nullptr;
    QAction* m_quitAction = nullptr;

    QTimer* m_timer = nullptr;
    PackageManager* m_pm = nullptr;

    bool m_trayEnabled = true;
    bool m_closeToTray = true;
    bool m_autostart = false;
    int m_checkIntervalHours = 6;
    int m_notifyThreshold = 1;
    bool m_autoUpdateFlatpak = false;
    bool m_autoUpdatePacman = false;
    bool m_autoUpdateAur = false;
    bool m_useCaelestiaUpdate = true;
    bool m_autoUpdateCaelestia = false;

    int m_pendingUpdateCount = 0;
    bool m_isChecking = false;
    QDateTime m_lastCheckTime;
};
