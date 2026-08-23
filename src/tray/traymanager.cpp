#include "traymanager.hpp"
#include "marketplace/packagemanager.hpp"
#include "themewatcher.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDebug>

TrayManager* TrayManager::instance() {
    static TrayManager inst;
    return &inst;
}

TrayManager* TrayManager::create(QQmlEngine*, QJSEngine*) {
    return instance();
}

TrayManager::TrayManager(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this)) {
    QSettings settings(QStringLiteral("AstraMarket"), QStringLiteral("astra"));
    m_trayEnabled = settings.value(QStringLiteral("tray/enabled"), true).toBool();
    m_closeToTray = settings.value(QStringLiteral("tray/closeToTray"), true).toBool();
    m_autostart = settings.value(QStringLiteral("tray/autostart"), checkAutostartFileExists()).toBool();
    m_checkIntervalHours = settings.value(QStringLiteral("tray/checkIntervalHours"), 6).toInt();
    m_notifyThreshold = settings.value(QStringLiteral("tray/notifyThreshold"), 1).toInt();
    m_autoUpdateFlatpak = settings.value(QStringLiteral("tray/autoUpdateFlatpak"), false).toBool();
    m_autoUpdatePacman = settings.value(QStringLiteral("tray/autoUpdatePacman"), false).toBool();
    m_autoUpdateAur = settings.value(QStringLiteral("tray/autoUpdateAur"), false).toBool();
    m_useCaelestiaUpdate = settings.value(QStringLiteral("plugins/useCaelestiaUpdate"), !QStandardPaths::findExecutable(QStringLiteral("caelestia")).isEmpty()).toBool();
    m_autoUpdateCaelestia = settings.value(QStringLiteral("tray/autoUpdateCaelestia"), false).toBool();

    initTray();

    connect(m_timer, &QTimer::timeout, this, &TrayManager::onScheduledCheckTimeout);
    restartTimer();
}

TrayManager::~TrayManager() {
    if (m_trayIcon) {
        m_trayIcon->hide();
    }
}

void TrayManager::setPackageManager(PackageManager* pm) {
    if (m_pm == pm)
        return;

    if (m_pm) {
        disconnect(m_pm, &PackageManager::updatesCompleted, this, &TrayManager::onUpdatesRefreshed);
    }

    m_pm = pm;

    if (m_pm) {
        connect(m_pm, &PackageManager::updatesCompleted, this, &TrayManager::onUpdatesRefreshed);
        connect(m_pm, &PackageManager::useCaelestiaUpdateChanged, this, [this] {
            const bool value = m_pm->useCaelestiaUpdate();
            if (m_useCaelestiaUpdate == value) return;
            m_useCaelestiaUpdate = value;
            emit useCaelestiaUpdateChanged();
        });

        if (m_useCaelestiaUpdate != m_pm->useCaelestiaUpdate()) {
            m_useCaelestiaUpdate = m_pm->useCaelestiaUpdate();
            emit useCaelestiaUpdateChanged();
        }
    }
}

QString TrayManager::lastCheckString() const {
    if (!m_lastCheckTime.isValid()) {
        return QStringLiteral("Never");
    }
    return m_lastCheckTime.toString(QStringLiteral("hh:mm - dd MMM"));
}

void TrayManager::updateTrayIcon() {
    if (!m_trayIcon)
        return;

    const bool isLight = ThemeWatcher::instance()->isLight();
    // tray-dark.svg has dark fill (#303945) for light theme / light panels
    // tray-light.svg has light fill (#EBF4FF) for dark theme / dark panels
    const QString primaryRes = isLight ? QStringLiteral(":/assets/icons/tray-dark.svg")
                                       : QStringLiteral(":/assets/icons/tray-light.svg");
    const QString fallbackFile = isLight ? QStringLiteral("assets/icons/tray-dark.svg")
                                         : QStringLiteral("assets/icons/tray-light.svg");

    QIcon icon;
    if (QFile::exists(primaryRes)) {
        icon = QIcon(primaryRes);
    } else if (QFile::exists(fallbackFile)) {
        icon = QIcon(fallbackFile);
    } else if (QFile::exists(QStringLiteral(":/assets/icons/AstraMarket.svg"))) {
        icon = QIcon(QStringLiteral(":/assets/icons/AstraMarket.svg"));
    } else {
        icon = QIcon(QStringLiteral("assets/icons/AstraMarket.svg"));
    }

    m_trayIcon->setIcon(icon);
}

void TrayManager::initTray() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning() << "TrayManager: System tray is not available on this system.";
        return;
    }

    m_trayIcon = new QSystemTrayIcon(this);
    updateTrayIcon();
    m_trayIcon->setToolTip(QStringLiteral("Astra Market"));

    connect(ThemeWatcher::instance(), &ThemeWatcher::themeChanged, this, &TrayManager::updateTrayIcon);

    m_trayMenu = new QMenu();

    m_statusAction = m_trayMenu->addAction(QStringLiteral("Astra Market"));
    m_statusAction->setEnabled(false);
    m_trayMenu->addSeparator();

    m_openAction = m_trayMenu->addAction(QStringLiteral("Open Astra Market"), this, &TrayManager::requestShowMainWindow);
    m_checkAction = m_trayMenu->addAction(QStringLiteral("Check for Updates"), this, &TrayManager::checkForUpdates);
    m_updateAllAction = m_trayMenu->addAction(QStringLiteral("Update All Packages"), this, &TrayManager::updateAll);
    m_updateAllAction->setEnabled(false);
    m_settingsAction = m_trayMenu->addAction(QStringLiteral("Preferences..."), this, &TrayManager::requestShowSettingsPage);

    m_trayMenu->addSeparator();
    m_quitAction = m_trayMenu->addAction(QStringLiteral("Quit"), QCoreApplication::instance(), &QCoreApplication::quit);

    m_trayIcon->setContextMenu(m_trayMenu);

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &TrayManager::onTrayIconActivated);
    connect(m_trayIcon, &QSystemTrayIcon::messageClicked, this, &TrayManager::requestShowUpdatesPage);

    if (m_trayEnabled) {
        m_trayIcon->show();
    }
}

void TrayManager::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        emit requestToggleMainWindow();
        break;
    case QSystemTrayIcon::DoubleClick:
        emit requestShowMainWindow();
        break;
    case QSystemTrayIcon::MiddleClick:
        emit requestShowUpdatesPage();
        break;
    default:
        break;
    }
}

void TrayManager::setTrayEnabled(bool enabled) {
    if (m_trayEnabled != enabled) {
        m_trayEnabled = enabled;
        QSettings settings(QStringLiteral("AstraMarket"), QStringLiteral("astra"));
        settings.setValue(QStringLiteral("tray/enabled"), enabled);
        emit trayEnabledChanged();

        if (m_trayIcon) {
            if (enabled) {
                m_trayIcon->show();
            } else {
                m_trayIcon->hide();
            }
        }
    }
}

void TrayManager::setCloseToTray(bool enabled) {
    if (m_closeToTray != enabled) {
        m_closeToTray = enabled;
        QSettings settings(QStringLiteral("AstraMarket"), QStringLiteral("astra"));
        settings.setValue(QStringLiteral("tray/closeToTray"), enabled);
        emit closeToTrayChanged();
    }
}

void TrayManager::setAutostart(bool enabled) {
    if (m_autostart != enabled) {
        m_autostart = enabled;
        QSettings settings(QStringLiteral("AstraMarket"), QStringLiteral("astra"));
        settings.setValue(QStringLiteral("tray/autostart"), enabled);
        updateAutostartFile(enabled);
        emit autostartChanged();
    }
}

void TrayManager::setCheckIntervalHours(int hours) {
    if (m_checkIntervalHours != hours) {
        m_checkIntervalHours = hours;
        QSettings settings(QStringLiteral("AstraMarket"), QStringLiteral("astra"));
        settings.setValue(QStringLiteral("tray/checkIntervalHours"), hours);
        emit checkIntervalHoursChanged();
        restartTimer();
    }
}

void TrayManager::setNotifyThreshold(int count) {
    if (m_notifyThreshold != count) {
        m_notifyThreshold = count;
        QSettings settings(QStringLiteral("AstraMarket"), QStringLiteral("astra"));
        settings.setValue(QStringLiteral("tray/notifyThreshold"), count);
        emit notifyThresholdChanged();
    }
}

void TrayManager::setAutoUpdateFlatpak(bool enabled) {
    if (m_autoUpdateFlatpak != enabled) {
        m_autoUpdateFlatpak = enabled;
        QSettings settings(QStringLiteral("AstraMarket"), QStringLiteral("astra"));
        settings.setValue(QStringLiteral("tray/autoUpdateFlatpak"), enabled);
        emit autoUpdateFlatpakChanged();
    }
}

void TrayManager::setAutoUpdatePacman(bool enabled) {
    if (m_autoUpdatePacman != enabled) {
        m_autoUpdatePacman = enabled;
        QSettings settings(QStringLiteral("AstraMarket"), QStringLiteral("astra"));
        settings.setValue(QStringLiteral("tray/autoUpdatePacman"), enabled);
        emit autoUpdatePacmanChanged();
    }
}

void TrayManager::setAutoUpdateAur(bool enabled) {
    if (m_autoUpdateAur != enabled) {
        m_autoUpdateAur = enabled;
        QSettings settings(QStringLiteral("AstraMarket"), QStringLiteral("astra"));
        settings.setValue(QStringLiteral("tray/autoUpdateAur"), enabled);
        emit autoUpdateAurChanged();
    }
}

void TrayManager::setUseCaelestiaUpdate(bool enabled) {
    if (m_useCaelestiaUpdate == enabled) return;

    m_useCaelestiaUpdate = enabled;
    if (m_pm) {
        m_pm->setUseCaelestiaUpdate(enabled);
    } else {
        QSettings settings(QStringLiteral("AstraMarket"), QStringLiteral("astra"));
        settings.setValue(QStringLiteral("plugins/useCaelestiaUpdate"), enabled);
    }
    emit useCaelestiaUpdateChanged();
}

void TrayManager::setAutoUpdateCaelestia(bool enabled) {
    if (m_autoUpdateCaelestia != enabled) {
        m_autoUpdateCaelestia = enabled;
        QSettings settings(QStringLiteral("AstraMarket"), QStringLiteral("astra"));
        settings.setValue(QStringLiteral("tray/autoUpdateCaelestia"), enabled);
        emit autoUpdateCaelestiaChanged();
    }
}

void TrayManager::restartTimer() {
    if (m_checkIntervalHours > 0) {
        const qint64 intervalMs = static_cast<qint64>(m_checkIntervalHours) * 3600 * 1000;
        m_timer->start(intervalMs);
    } else {
        m_timer->stop();
    }
}

void TrayManager::onScheduledCheckTimeout() {
    checkForUpdates();
}

void TrayManager::checkForUpdates() {
    if (m_isChecking)
        return;

    m_isChecking = true;
    emit isCheckingChanged();

    if (m_pm) {
        m_pm->checkForUpdatesAsync();
    }
}

void TrayManager::onUpdatesRefreshed() {
    m_isChecking = false;
    emit isCheckingChanged();

    m_lastCheckTime = QDateTime::currentDateTime();
    emit lastCheckStringChanged();

    if (!m_pm)
        return;

    const QVariantList updates = m_pm->checkForUpdates();
    m_pendingUpdateCount = updates.size();
    emit pendingUpdateCountChanged();

    updateTrayIconState();

    int flatpakCount = 0;
    int pacmanCount = 0;
    int aurCount = 0;
    for (const QVariant& u : updates) {
        const QVariantMap map = u.toMap();
        const QString backend = map.value(QStringLiteral("backend")).toString().toLower();
        if (backend.contains(QStringLiteral("flatpak"))) flatpakCount++;
        else if (backend.contains(QStringLiteral("pacman"))) pacmanCount++;
        else if (backend.contains(QStringLiteral("aur"))) aurCount++;
    }

    QStringList availableSources;
    if (flatpakCount > 0) availableSources.append(QStringLiteral("Flatpak"));
    if (m_useCaelestiaUpdate) {
        if (pacmanCount > 0 || aurCount > 0) availableSources.append(QStringLiteral("Caelestia (System/AUR)"));
    } else {
        if (pacmanCount > 0) availableSources.append(QStringLiteral("Pacman"));
        if (aurCount > 0) availableSources.append(QStringLiteral("AUR"));
    }

    QStringList autoUpdatingSources;
    if (m_autoUpdateFlatpak && flatpakCount > 0) {
        autoUpdatingSources.append(QStringLiteral("Flatpak"));
    }
    if (m_useCaelestiaUpdate) {
        if (m_autoUpdateCaelestia && (pacmanCount > 0 || aurCount > 0)) {
            autoUpdatingSources.append(QStringLiteral("Caelestia"));
        }
    } else {
        if (m_autoUpdatePacman && pacmanCount > 0) autoUpdatingSources.append(QStringLiteral("Pacman"));
        if (m_autoUpdateAur && aurCount > 0) autoUpdatingSources.append(QStringLiteral("AUR"));
    }

    if (m_pendingUpdateCount >= m_notifyThreshold && m_pendingUpdateCount > 0) {
        if (!autoUpdatingSources.isEmpty()) {
            const QString title = QStringLiteral("Astra Market - Applying Updates");
            const QString message = QStringLiteral("Automatically applying updates for: %1 (%2 total pending update%3).")
                .arg(autoUpdatingSources.join(QStringLiteral(", ")))
                .arg(m_pendingUpdateCount)
                .arg(m_pendingUpdateCount > 1 ? QStringLiteral("s") : QString());
            showNotification(title, message);
        } else {
            const QString title = QStringLiteral("Astra Market - Updates Available");
            const QString message = QStringLiteral("%1 update%2 available across: %3.")
                .arg(m_pendingUpdateCount)
                .arg(m_pendingUpdateCount > 1 ? QStringLiteral("s") : QString())
                .arg(availableSources.isEmpty() ? QStringLiteral("System") : availableSources.join(QStringLiteral(", ")));
            showNotification(title, message);
        }
    }

    if (!autoUpdatingSources.isEmpty()) {
        m_pm->updateAllPackages();
    }
}

void TrayManager::updateTrayIconState() {
    if (!m_trayIcon)
        return;

    if (m_pendingUpdateCount > 0) {
        m_trayIcon->setToolTip(QStringLiteral("Astra Market - %1 update%2 available")
            .arg(m_pendingUpdateCount)
            .arg(m_pendingUpdateCount > 1 ? QStringLiteral("s") : QString()));
        if (m_statusAction) {
            m_statusAction->setText(QStringLiteral("Updates Available: %1").arg(m_pendingUpdateCount));
        }
        if (m_updateAllAction) {
            m_updateAllAction->setEnabled(true);
        }
    } else {
        m_trayIcon->setToolTip(QStringLiteral("Astra Market - System Up to Date"));
        if (m_statusAction) {
            m_statusAction->setText(QStringLiteral("System Up to Date"));
        }
        if (m_updateAllAction) {
            m_updateAllAction->setEnabled(false);
        }
    }
}

void TrayManager::updateAll() {
    if (m_pm) {
        m_pm->updateAllPackages();
    }
}

void TrayManager::showMainWindow() {
    emit requestShowMainWindow();
}

void TrayManager::toggleMainWindow() {
    emit requestToggleMainWindow();
}

void TrayManager::showNotification(const QString& title, const QString& message) {
    if (m_trayIcon && m_trayIcon->isVisible()) {
        m_trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 8000);
    }

    if (!QStandardPaths::findExecutable(QStringLiteral("notify-send")).isEmpty()) {
        QProcess::startDetached(QStringLiteral("notify-send"), {
            QStringLiteral("-a"), QStringLiteral("Astra Market"),
            QStringLiteral("-i"), QStringLiteral("AstraMarket"),
            title,
            message
        });
    }
}

bool TrayManager::checkAutostartFileExists() const {
    const QString autostartDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/autostart");
    return QFile::exists(autostartDir + QStringLiteral("/astra-tray.desktop"));
}

void TrayManager::updateAutostartFile(bool enable) {
    const QString autostartDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/autostart");
    const QString desktopFile = autostartDir + QStringLiteral("/astra-tray.desktop");

    if (enable) {
        QDir().mkpath(autostartDir);
        QFile file(desktopFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            const QString content = QStringLiteral(
                "[Desktop Entry]\n"
                "Type=Application\n"
                "Name=Astra Market Tray\n"
                "Comment=Unified package manager background notifier\n"
                "Exec=astra --tray\n"
                "Icon=AstraMarket\n"
                "Terminal=false\n"
                "Categories=System;PackageManager;\n"
                "X-GNOME-Autostart-enabled=true\n"
            );
            file.write(content.toUtf8());
            file.close();
        }
    } else {
        if (QFile::exists(desktopFile)) {
            QFile::remove(desktopFile);
        }
    }
}
