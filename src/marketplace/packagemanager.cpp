#include "packagemanager.hpp"
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

PackageManager::PackageManager(QObject* parent) : QObject(parent) {
    initPlugins();
}

PackageManager::~PackageManager() = default;

PackageManager* PackageManager::create(QQmlEngine*, QJSEngine*) {
    return new PackageManager();
}

void PackageManager::initPlugins() {
    m_flatpakPlugin = new FlatpakPlugin(this);
    m_pacmanPlugin = new PacmanPlugin(this);
    m_aurPlugin = new AurPlugin(this);
    m_appimagePlugin = new AppImagePlugin(this);

    registerPlugin(m_flatpakPlugin);
    registerPlugin(m_pacmanPlugin);
    registerPlugin(m_aurPlugin);
    registerPlugin(m_appimagePlugin);

    connect(m_flatpakPlugin, &IPackagePlugin::enabledChanged, this, &PackageManager::enableFlatpakChanged);
    connect(m_pacmanPlugin, &IPackagePlugin::enabledChanged, this, &PackageManager::enablePacmanChanged);
    connect(m_aurPlugin, &IPackagePlugin::enabledChanged, this, &PackageManager::enableAurChanged);
    connect(m_appimagePlugin, &IPackagePlugin::enabledChanged, this, &PackageManager::enableAppImageChanged);
    connect(m_aurPlugin, &IPackagePlugin::availabilityChanged, this, &PackageManager::aurHelperChanged);

    QStringList pluginDirs = {
        QDir::homePath() + QStringLiteral("/.config/astra/plugins"),
        QStringLiteral("/usr/share/astra/plugins")
    };
    QList<ScriptablePlugin*> scriptPlugins = ScriptablePlugin::loadFromDirectories(pluginDirs, this);
    for (ScriptablePlugin* sp : scriptPlugins) {
        registerPlugin(sp);
    }
}

void PackageManager::registerPlugin(IPackagePlugin* plugin) {
    if (plugin && !m_plugins.contains(plugin)) {
        m_plugins.append(plugin);
        emit pluginsChanged();
    }
}

IPackagePlugin* PackageManager::findPlugin(const QString& backendOrId) const {
    QString target = backendOrId.trimmed().toLower();
    for (IPackagePlugin* p : m_plugins) {
        if (p->id().compare(target, Qt::CaseInsensitive) == 0 ||
            p->name().compare(target, Qt::CaseInsensitive) == 0) {
            return p;
        }
    }
    return nullptr;
}

QVariantList PackageManager::getRegisteredPluginsInfo() const {
    QVariantList list;
    for (IPackagePlugin* p : m_plugins) {
        QVariantMap map;
        map[QStringLiteral("id")] = p->id();
        map[QStringLiteral("name")] = p->name();
        map[QStringLiteral("description")] = p->description();
        map[QStringLiteral("icon")] = p->icon();
        map[QStringLiteral("isAvailable")] = p->isAvailable();
        map[QStringLiteral("isEnabled")] = p->isEnabled();
        list.append(map);
    }
    return list;
}

bool PackageManager::enableFlatpak() const { return m_flatpakPlugin ? m_flatpakPlugin->isEnabled() : false; }
bool PackageManager::enablePacman() const { return m_pacmanPlugin ? m_pacmanPlugin->isEnabled() : false; }
bool PackageManager::enableAur() const { return m_aurPlugin ? m_aurPlugin->isEnabled() : false; }
bool PackageManager::enableAppImage() const { return m_appimagePlugin ? m_appimagePlugin->isEnabled() : false; }
QString PackageManager::aurHelper() const { return m_aurPlugin ? m_aurPlugin->aurHelper() : QStringLiteral("auto"); }

bool PackageManager::isFlatpakAvailable() const { return m_flatpakPlugin ? m_flatpakPlugin->isAvailable() : false; }
bool PackageManager::isPacmanAvailable() const { return m_pacmanPlugin ? m_pacmanPlugin->isAvailable() : false; }
bool PackageManager::isAurAvailable() const { return m_aurPlugin ? m_aurPlugin->isAvailable() : false; }
bool PackageManager::isAppImageAvailable() const { return m_appimagePlugin ? m_appimagePlugin->isAvailable() : false; }

void PackageManager::setEnableFlatpak(bool enable) { if (m_flatpakPlugin) m_flatpakPlugin->setEnabled(enable); }
void PackageManager::setEnablePacman(bool enable) { if (m_pacmanPlugin) m_pacmanPlugin->setEnabled(enable); }
void PackageManager::setEnableAur(bool enable) { if (m_aurPlugin) m_aurPlugin->setEnabled(enable); }
void PackageManager::setEnableAppImage(bool enable) { if (m_appimagePlugin) m_appimagePlugin->setEnabled(enable); }
void PackageManager::setAurHelper(const QString& helper) { if (m_aurPlugin) m_aurPlugin->setAurHelper(helper); }

void PackageManager::setBusy(bool busy) {
    if (m_isBusy != busy) {
        m_isBusy = busy;
        emit isBusyChanged();
    }
}

void PackageManager::setStatusMessage(const QString& message) {
    if (m_statusMessage != message) {
        m_statusMessage = message;
        emit statusMessageChanged();
    }
}

QList<QVariantMap> PackageManager::getInstallSources(const QString& backend, const QString& packageId) {
    IPackagePlugin* plugin = findPlugin(backend);
    if (plugin) {
        return plugin->getInstallSources(packageId);
    }
    return {};
}

void PackageManager::searchPackagesAsync(const QString& query, const QString& sourceFilter) {
    setBusy(true);
    auto* watcher = new QFutureWatcher<QVariantList>(this);
    connect(watcher, &QFutureWatcher<QVariantList>::finished, this, [this, watcher] {
        QVariantList results = watcher->result();
        watcher->deleteLater();
        setBusy(false);
        emit searchCompleted(results);
    });

    QFuture<QVariantList> future = QtConcurrent::run([this, query, sourceFilter] {
        return searchPackages(query, sourceFilter);
    });
    watcher->setFuture(future);
}

QVariantList PackageManager::searchPackages(const QString& query, const QString& sourceFilter) {
    QVariantList results;
    if (query.trimmed().isEmpty()) return results;

    QSet<QString> seenKeys;
    for (IPackagePlugin* plugin : m_plugins) {
        if (!plugin->isAvailable() || !plugin->isEnabled()) continue;
        if (!sourceFilter.isEmpty() && plugin->id().compare(sourceFilter, Qt::CaseInsensitive) != 0 && plugin->name().compare(sourceFilter, Qt::CaseInsensitive) != 0) {
            continue;
        }

        QVariantList pResults = plugin->search(query);
        for (const QVariant& v : pResults) {
            QVariantMap item = v.toMap();
            QString key = item.value(QStringLiteral("backend")).toString() + QStringLiteral(":") + item.value(QStringLiteral("id")).toString();
            if (seenKeys.contains(key)) continue;
            seenKeys.insert(key);
            results.append(item);
        }
    }
    return results;
}

void PackageManager::getInstalledPackagesAsync() {
    setBusy(true);
    auto* watcher = new QFutureWatcher<QVariantList>(this);
    connect(watcher, &QFutureWatcher<QVariantList>::finished, this, [this, watcher] {
        QVariantList results = watcher->result();
        watcher->deleteLater();
        setBusy(false);
        emit installedCompleted(results);
    });

    QFuture<QVariantList> future = QtConcurrent::run([this] {
        return getInstalledPackages();
    });
    watcher->setFuture(future);
}

QVariantList PackageManager::getInstalledPackages() {
    QVariantList results;
    for (IPackagePlugin* plugin : m_plugins) {
        if (!plugin->isAvailable() || !plugin->isEnabled()) continue;
        results.append(plugin->getInstalled());
    }
    return results;
}

void PackageManager::installPackage(const QString& backend, const QString& packageId, const QString& scope) {
    setBusy(true);
    setStatusMessage(QStringLiteral("Installing ") + packageId + QStringLiteral("..."));

    auto* watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, packageId] {
        bool success = watcher->result();
        watcher->deleteLater();
        setBusy(false);
        setStatusMessage(QString());
        emit operationFinished(success, success ? QStringLiteral("Successfully installed ") + packageId : QStringLiteral("Failed to install ") + packageId);
    });

    QVariantMap opts;
    opts[QStringLiteral("scope")] = scope;

    QFuture<bool> future = QtConcurrent::run([this, backend, packageId, opts] {
        IPackagePlugin* plugin = findPlugin(backend);
        if (!plugin) return false;
        return plugin->install(packageId, opts, [this, packageId](int pct, const QString& status) {
            emit operationProgress(packageId, pct, status);
        });
    });
    watcher->setFuture(future);
}

void PackageManager::uninstallPackage(const QString& backend, const QString& packageId, const QString& scope) {
    setBusy(true);
    setStatusMessage(QStringLiteral("Uninstalling ") + packageId + QStringLiteral("..."));

    auto* watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, packageId] {
        bool success = watcher->result();
        watcher->deleteLater();
        setBusy(false);
        setStatusMessage(QString());
        emit operationFinished(success, success ? QStringLiteral("Successfully uninstalled ") + packageId : QStringLiteral("Failed to uninstall ") + packageId);
    });

    QVariantMap opts;
    opts[QStringLiteral("scope")] = scope;

    QFuture<bool> future = QtConcurrent::run([this, backend, packageId, opts] {
        IPackagePlugin* plugin = findPlugin(backend);
        if (!plugin) return false;
        return plugin->uninstall(packageId, opts, [this, packageId](int pct, const QString& status) {
            emit operationProgress(packageId, pct, status);
        });
    });
    watcher->setFuture(future);
}

void PackageManager::launchApp(const QString& backend, const QString& packageId, const QString& execPath) {
    Q_UNUSED(execPath);
    IPackagePlugin* plugin = findPlugin(backend);
    if (plugin) {
        plugin->launch(packageId);
    }
}

QString PackageManager::getIconPath(const QString& iconName, const QString& backend) {
    Q_UNUSED(backend);
    if (iconName.isEmpty()) return QString();
    return QStringLiteral("image://icon/") + iconName;
}

QVariantMap PackageManager::getPackageDetails(const QString& packageId, const QString& backend) {
    IPackagePlugin* plugin = findPlugin(backend);
    if (plugin) {
        return plugin->getDetails(packageId);
    }
    return {};
}

void PackageManager::fetchPackageDetailsAsync(const QString& packageId, const QString& backend) {
    auto* watcher = new QFutureWatcher<QVariantMap>(this);
    connect(watcher, &QFutureWatcher<QVariantMap>::finished, this, [this, watcher] {
        QVariantMap details = watcher->result();
        watcher->deleteLater();
        emit packageDetailsReady(details);
    });

    QFuture<QVariantMap> future = QtConcurrent::run([this, packageId, backend] {
        return getPackageDetails(packageId, backend);
    });
    watcher->setFuture(future);
}

QVariantList PackageManager::checkForUpdates() {
    QVariantList results;
    for (IPackagePlugin* plugin : m_plugins) {
        if (!plugin->isAvailable() || !plugin->isEnabled()) continue;
        results.append(plugin->getUpdates());
    }
    return results;
}

void PackageManager::checkForUpdatesAsync() {
    setBusy(true);
    auto* watcher = new QFutureWatcher<QVariantList>(this);
    connect(watcher, &QFutureWatcher<QVariantList>::finished, this, [this, watcher] {
        QVariantList results = watcher->result();
        watcher->deleteLater();
        setBusy(false);
        emit updatesCompleted(results);
    });

    QFuture<QVariantList> future = QtConcurrent::run([this] {
        return checkForUpdates();
    });
    watcher->setFuture(future);
}

void PackageManager::updateAllPackages() {
    setBusy(true);
    setStatusMessage(QStringLiteral("Updating all packages..."));

    auto* watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher] {
        watcher->deleteLater();
        setBusy(false);
        setStatusMessage(QString());
        checkForUpdatesAsync();
    });

    QFuture<bool> future = QtConcurrent::run([this] {
        QProcess proc;
        proc.start(QStringLiteral("flatpak"), {QStringLiteral("update"), QStringLiteral("-y")});
        proc.waitForFinished(-1);

        QProcess pacProc;
        pacProc.start(QStringLiteral("pkexec"), {QStringLiteral("pacman"), QStringLiteral("-Syu"), QStringLiteral("--noconfirm")});
        pacProc.waitForFinished(-1);
        return true;
    });
    watcher->setFuture(future);
}
