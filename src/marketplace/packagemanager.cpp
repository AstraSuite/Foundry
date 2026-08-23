#include "packagemanager.hpp"
#include "pluginprocess.hpp"
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTime>

PackageManager::PackageManager(QObject* parent) : QObject(parent) {
    m_pluginPool.setMaxThreadCount(8);
    initPlugins();
}

PackageManager::~PackageManager() = default;

PackageManager* PackageManager::create(QQmlEngine*, QJSEngine*) {
    return new PackageManager();
}

#include <QSettings>

void PackageManager::initPlugins() {
    m_flatpakPlugin = new FlatpakPlugin(this);
    m_pacmanPlugin = new PacmanPlugin(this);
    m_aurPlugin = new AurPlugin(this);
    m_appimagePlugin = new AppImagePlugin(this);

    QSettings settings(QStringLiteral("AstraMarket"), QStringLiteral("astra"));
    m_flatpakPlugin->setEnabled(settings.value(QStringLiteral("plugins/flatpak"), true).toBool());
    m_pacmanPlugin->setEnabled(settings.value(QStringLiteral("plugins/pacman"), true).toBool());
    m_aurPlugin->setEnabled(settings.value(QStringLiteral("plugins/aur"), true).toBool());
    m_appimagePlugin->setEnabled(settings.value(QStringLiteral("plugins/appimage"), true).toBool());

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
bool PackageManager::isCaelestiaAvailable() const { return !QStandardPaths::findExecutable(QStringLiteral("caelestia")).isEmpty(); }

bool PackageManager::useCaelestiaUpdate() const {
    QSettings settings(QStringLiteral("AstraMarket"), QStringLiteral("astra"));
    return settings.value(QStringLiteral("plugins/useCaelestiaUpdate"), isCaelestiaAvailable()).toBool();
}

void PackageManager::setUseCaelestiaUpdate(bool enable) {
    QSettings settings(QStringLiteral("AstraMarket"), QStringLiteral("astra"));
    if (settings.value(QStringLiteral("plugins/useCaelestiaUpdate")).toBool() != enable) {
        settings.setValue(QStringLiteral("plugins/useCaelestiaUpdate"), enable);
        emit useCaelestiaUpdateChanged();
    }
}

void PackageManager::setEnableFlatpak(bool enable) {
    if (m_flatpakPlugin) {
        m_flatpakPlugin->setEnabled(enable);
        QSettings settings(QStringLiteral("AstraMarket"), QStringLiteral("astra"));
        settings.setValue(QStringLiteral("plugins/flatpak"), enable);
    }
}
void PackageManager::setEnablePacman(bool enable) {
    if (m_pacmanPlugin) {
        m_pacmanPlugin->setEnabled(enable);
        QSettings settings(QStringLiteral("AstraMarket"), QStringLiteral("astra"));
        settings.setValue(QStringLiteral("plugins/pacman"), enable);
    }
}
void PackageManager::setEnableAur(bool enable) {
    if (m_aurPlugin) {
        m_aurPlugin->setEnabled(enable);
        QSettings settings(QStringLiteral("AstraMarket"), QStringLiteral("astra"));
        settings.setValue(QStringLiteral("plugins/aur"), enable);
    }
}
void PackageManager::setEnableAppImage(bool enable) {
    if (m_appimagePlugin) {
        m_appimagePlugin->setEnabled(enable);
        QSettings settings(QStringLiteral("AstraMarket"), QStringLiteral("astra"));
        settings.setValue(QStringLiteral("plugins/appimage"), enable);
    }
}
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

#include <algorithm>

void PackageManager::searchPackagesAsync(const QString& query, const QString& sourceFilter) {
    quint64 seq = ++m_searchSequence;
    setBusy(true);
    auto* watcher = new QFutureWatcher<QVariantList>(this);
    connect(watcher, &QFutureWatcher<QVariantList>::finished, this, [this, watcher, seq] {
        QVariantList results = watcher->result();
        watcher->deleteLater();
        if (seq == m_searchSequence) {
            setBusy(false);
            emit searchCompleted(results);
        }
    });

    QFuture<QVariantList> future = QtConcurrent::run([this, query, sourceFilter] {
        return searchPackages(query, sourceFilter);
    });
    watcher->setFuture(future);
}

QVariantList PackageManager::searchPackages(const QString& query, const QString& sourceFilter) {
    QVariantList results;
    QString q = query.trimmed();
    if (q.isEmpty()) return results;
    QString qLower = q.toLower();

    QSet<QString> seenKeys;
    struct ScoredItem {
        QVariantMap item;
        int score{0};
    };
    QList<ScoredItem> scoredItems;

    QList<IPackagePlugin*> eligiblePlugins;
    for (IPackagePlugin* plugin : m_plugins) {
        if (!plugin->isAvailable() || !plugin->isEnabled()) continue;
        if (!sourceFilter.isEmpty() &&
            plugin->id().compare(sourceFilter, Qt::CaseInsensitive) != 0 &&
            plugin->name().compare(sourceFilter, Qt::CaseInsensitive) != 0) {
            continue;
        }
        eligiblePlugins.append(plugin);
    }
    if (eligiblePlugins.isEmpty()) return results;

    QList<QVariantList> pluginResults;
    if (eligiblePlugins.size() == 1) {
        pluginResults.append(eligiblePlugins.first()->search(q));
    } else {
        pluginResults = QtConcurrent::blockingMapped<QList<QVariantList>>(&m_pluginPool, eligiblePlugins, [&q](IPackagePlugin* plugin) {
            return plugin->search(q);
        });
    }

    for (const QVariantList& pResults : pluginResults) {
        for (const QVariant& v : pResults) {
            QVariantMap item = v.toMap();
            QString backend = item.value(QStringLiteral("backend")).toString();
            QString id = item.value(QStringLiteral("id")).toString();
            QString key = backend + QStringLiteral(":") + id;
            if (seenKeys.contains(key)) continue;
            seenKeys.insert(key);

            QString name = item.value(QStringLiteral("name")).toString();
            QString summary = item.value(QStringLiteral("summary")).toString();
            QString nameLower = name.toLower();
            QString idLower = id.toLower();
            QString summaryLower = summary.toLower();

            int score = 0;

            // 1. Exact matches
            if (nameLower == qLower) {
                score += 10000;
            } else if (idLower == qLower) {
                score += 9000;
            } else if (idLower.endsWith(QStringLiteral(".") + qLower) || idLower.startsWith(qLower + QStringLiteral("."))) {
                score += 7000;
            }

            // 2. Starts with / prefix
            if (nameLower.startsWith(qLower)) {
                score += 5000;
            } else if (idLower.startsWith(qLower) || idLower.contains(QStringLiteral(".") + qLower)) {
                score += 3500;
            }

            // 3. Word boundary or contains
            if (nameLower.contains(QStringLiteral(" ") + qLower) || nameLower.contains(QStringLiteral("-") + qLower)) {
                score += 2500;
            } else if (nameLower.contains(qLower)) {
                score += 1500;
            }

            if (idLower.contains(qLower)) {
                score += 800;
            }

            if (summaryLower.contains(qLower)) {
                score += 200;
                if (summaryLower.startsWith(qLower)) {
                    score += 300;
                }
            }

            // Verified / official boost
            if (item.value(QStringLiteral("verified")).toBool()) {
                score += 500;
            }

            // Backend priority bonus: Flatpak official apps generally user-friendly
            if (backend == QStringLiteral("Flatpak") && (nameLower.contains(qLower) || idLower.contains(qLower))) {
                score += 150;
            }

            // Shorter names matching query are usually the primary package
            if (nameLower.contains(qLower)) {
                score -= qMin(name.length() * 5, 200);
            }

            scoredItems.append({item, score});
        }
    }

    // Sort by score descending
    std::stable_sort(scoredItems.begin(), scoredItems.end(), [](const ScoredItem& a, const ScoredItem& b) {
        return a.score > b.score;
    });

    for (const auto& s : scoredItems) {
        results.append(s.item);
    }
    return results;
}

void PackageManager::getInstalledPackagesAsync() {
    quint64 seq = ++m_installedSequence;
    setBusy(true);
    auto* watcher = new QFutureWatcher<QVariantList>(this);
    connect(watcher, &QFutureWatcher<QVariantList>::finished, this, [this, watcher, seq] {
        QVariantList results = watcher->result();
        watcher->deleteLater();
        if (seq == m_installedSequence) {
            setBusy(false);
            emit installedCompleted(results);
        }
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

bool PackageManager::isPackageInstalled(const QString& backend, const QString& packageId) {
    if (packageId.isEmpty()) return false;
    IPackagePlugin* plugin = findPlugin(backend);
    if (plugin) {
        QVariantList installed = plugin->getInstalled();
        for (const QVariant& v : installed) {
            QVariantMap map = v.toMap();
            if (map.value(QStringLiteral("id")).toString().compare(packageId, Qt::CaseInsensitive) == 0 ||
                map.value(QStringLiteral("name")).toString().compare(packageId, Qt::CaseInsensitive) == 0) {
                return true;
            }
        }
    }
    return false;
}

void PackageManager::appendLog(const QString& line) {
    if (QThread::currentThread() != this->thread()) {
        QMetaObject::invokeMethod(this, [this, line]() {
            appendLog(line);
        }, Qt::QueuedConnection);
        return;
    }

    QString trimmed = line.trimmed();
    if (trimmed.isEmpty()) return;

    // Check if this is an in-progress update of the same step (e.g. "Installing 1/5...", "Downloading...")
    // to avoid appending 50 identical lines per download
    if (!m_logLines.isEmpty()) {
        const QString& last = m_logLines.last();
        QString lastContent = last.section(QLatin1Char(']'), 1).trimmed();
        QString currentContent = trimmed.section(QLatin1Char(']'), 1).trimmed();
        if (lastContent.isEmpty()) lastContent = last;
        if (currentContent.isEmpty()) currentContent = trimmed;

        QString lastPrefix = lastContent.section(QLatin1Char(' '), 0, 1);
        QString curPrefix = currentContent.section(QLatin1Char(' '), 0, 1);
        if (!lastPrefix.isEmpty() && lastPrefix == curPrefix &&
            (lastPrefix.contains(QLatin1String("Installing")) ||
             lastPrefix.contains(QLatin1String("Downloading")) ||
             lastPrefix.contains(QLatin1String("Fetching")))) {
            m_logLines.last() = trimmed;
            emit logLinesChanged();
            return;
        }
    }

    m_logLines.append(trimmed);
    if (m_logLines.size() > 3000) {
        m_logLines.removeFirst();
    }
    emit logLinesChanged();
}

void PackageManager::clearLogs() {
    m_logLines.clear();
    emit logLinesChanged();
}

void PackageManager::installPackage(const QString& backend, const QString& packageId, const QString& scope) {
    setBusy(true);
    m_currentProgress = 0;
    emit currentProgressChanged();
    QString statusText = QStringLiteral("Installing ") + packageId + QStringLiteral(" (") + backend + QStringLiteral(")...");
    setStatusMessage(statusText);
    appendLog(QStringLiteral("[%1] Starting installation: %2 [%3, scope: %4]").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")), packageId, backend, scope.isEmpty() ? QStringLiteral("default") : scope));

    auto* watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, packageId, backend] {
        bool success = watcher->result();
        watcher->deleteLater();
        setBusy(false);
        m_currentProgress = success ? 100 : 0;
        emit currentProgressChanged();
        setStatusMessage(QString());
        appendLog(QStringLiteral("[%1] %2: %3 (%4)").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")), success ? QStringLiteral("SUCCESS") : QStringLiteral("FAILED"), packageId, backend));
        emit operationFinished(success, success ? QStringLiteral("Successfully installed ") + packageId : QStringLiteral("Failed to install ") + packageId);
    });

    QVariantMap opts;
    if (!scope.isEmpty()) opts[QStringLiteral("scope")] = scope;

    QFuture<bool> future = QtConcurrent::run([this, backend, packageId, opts] {
        IPackagePlugin* plugin = findPlugin(backend);
        if (!plugin) return false;
        return plugin->install(packageId, opts, [this, packageId](int pct, const QString& status) {
            m_currentProgress = pct;
            emit currentProgressChanged();
            if (!status.isEmpty()) {
                appendLog(QStringLiteral("[%1] [%2%] %3").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")), QString::number(pct), status));
            }
            emit operationProgress(packageId, pct, status);
        });
    });
    watcher->setFuture(future);
}

void PackageManager::updatePackage(const QString& backend, const QString& packageId, const QString& scope) {
    setBusy(true);
    m_currentProgress = 0;
    emit currentProgressChanged();
    setStatusMessage(QStringLiteral("Updating ") + packageId + QStringLiteral(" (") + backend + QStringLiteral(")..."));
    appendLog(QStringLiteral("[%1] Starting update: %2 [%3]").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")), packageId, backend));

    if (backend == QStringLiteral("Pacman") || backend == QStringLiteral("AUR")) {
        appendLog(QStringLiteral("[%1] %2 is upgraded with a full sync (-Syu) to avoid a partial upgrade").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")), packageId));
    }

    auto* watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, packageId, backend] {
        const bool success = watcher->result();
        watcher->deleteLater();
        setBusy(false);
        m_currentProgress = success ? 100 : 0;
        emit currentProgressChanged();
        setStatusMessage(QString());
        appendLog(QStringLiteral("[%1] %2: %3 (%4)").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")), success ? QStringLiteral("SUCCESS") : QStringLiteral("FAILED"), packageId, backend));
        emit operationFinished(success, success ? QStringLiteral("Successfully updated ") + packageId : QStringLiteral("Failed to update ") + packageId);
    });

    QVariantMap opts;
    if (!scope.isEmpty()) opts[QStringLiteral("scope")] = scope;

    QFuture<bool> future = QtConcurrent::run([this, backend, packageId, opts] {
        IPackagePlugin* plugin = findPlugin(backend);
        if (!plugin) return false;
        return plugin->update(packageId, opts, [this, packageId](int pct, const QString& status) {
            m_currentProgress = pct;
            emit currentProgressChanged();
            if (!status.isEmpty()) {
                appendLog(QStringLiteral("[%1] [%2%] %3").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")), QString::number(pct), status));
            }
            emit operationProgress(packageId, pct, status);
        });
    });
    watcher->setFuture(future);
}

void PackageManager::uninstallPackage(const QString& backend, const QString& packageId, const QString& scope) {
    setBusy(true);
    m_currentProgress = 0;
    emit currentProgressChanged();
    QString statusText = QStringLiteral("Uninstalling ") + packageId + QStringLiteral(" (") + backend + QStringLiteral(")...");
    setStatusMessage(statusText);
    appendLog(QStringLiteral("[%1] Starting uninstallation: %2 [%3]").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")), packageId, backend));

    auto* watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher, packageId, backend] {
        bool success = watcher->result();
        watcher->deleteLater();
        setBusy(false);
        m_currentProgress = success ? 100 : 0;
        emit currentProgressChanged();
        setStatusMessage(QString());
        appendLog(QStringLiteral("[%1] %2: %3 (%4)").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")), success ? QStringLiteral("SUCCESS") : QStringLiteral("FAILED"), packageId, backend));
        emit operationFinished(success, success ? QStringLiteral("Successfully uninstalled ") + packageId : QStringLiteral("Failed to uninstall ") + packageId);
    });

    QVariantMap opts;
    if (!scope.isEmpty()) opts[QStringLiteral("scope")] = scope;

    QFuture<bool> future = QtConcurrent::run([this, backend, packageId, opts] {
        IPackagePlugin* plugin = findPlugin(backend);
        if (!plugin) return false;
        return plugin->uninstall(packageId, opts, [this, packageId](int pct, const QString& status) {
            m_currentProgress = pct;
            emit currentProgressChanged();
            if (!status.isEmpty()) {
                appendLog(QStringLiteral("[%1] [%2%] %3").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")), QString::number(pct), status));
            }
            emit operationProgress(packageId, pct, status);
        });
    });
    watcher->setFuture(future);
}

void PackageManager::launchApp(const QString& backend, const QString& packageId, const QString& execPath) {
    IPackagePlugin* plugin = findPlugin(backend);
    if (plugin) {
        plugin->launch(execPath.isEmpty() ? packageId : execPath);
    }
}

QString PackageManager::getIconPath(const QString& iconName, const QString& backend) {
    Q_UNUSED(backend);
    if (iconName.isEmpty()) return QString();
    if (iconName.startsWith(QLatin1String("http://")) ||
        iconName.startsWith(QLatin1String("https://")) ||
        iconName.startsWith(QLatin1String("image://")) ||
        iconName.startsWith(QLatin1String("file://")) ||
        iconName.startsWith(QLatin1String("qrc:/"))) {
        return iconName;
    }
    if (iconName.startsWith(QLatin1Char('/'))) {
        return QStringLiteral("file://") + iconName;
    }
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

void PackageManager::fetchCollectionAsync(const QString& collection, int limit) {
    if (collection.isEmpty() || !m_flatpakPlugin) return;

    if (m_collections.contains(collection)) {
        const QVariantList cached = m_collections.value(collection);
        QMetaObject::invokeMethod(this, [this, collection, cached] {
            emit collectionReady(collection, cached);
        }, Qt::QueuedConnection);
        return;
    }

    if (m_pendingCollections.contains(collection)) return;
    m_pendingCollections.insert(collection);

    auto* watcher = new QFutureWatcher<QVariantList>(this);
    connect(watcher, &QFutureWatcher<QVariantList>::finished, this, [this, watcher, collection] {
        const QVariantList apps = watcher->result();
        watcher->deleteLater();
        m_pendingCollections.remove(collection);
        if (!apps.isEmpty()) m_collections.insert(collection, apps);
        emit collectionReady(collection, apps);
    });

    watcher->setFuture(QtConcurrent::run([this, collection, limit] {
        return m_flatpakPlugin->getCollection(collection, limit);
    }));
}

QVariantList PackageManager::checkForUpdates() {
    QVariantList results;
    const bool useCaelestia = useCaelestiaUpdate();
    for (IPackagePlugin* plugin : m_plugins) {
        if (!plugin->isAvailable() || !plugin->isEnabled()) continue;
        if (useCaelestia && (plugin->id() == QStringLiteral("Pacman") || plugin->id() == QStringLiteral("AUR"))) {
            // When Caelestia CLI update is enabled, it handles system & AUR updates
            continue;
        }
        results.append(plugin->getUpdates());
    }
    return results;
}

void PackageManager::checkForUpdatesAsync() {
    quint64 seq = ++m_updatesSequence;
    setBusy(true);
    auto* watcher = new QFutureWatcher<QVariantList>(this);
    connect(watcher, &QFutureWatcher<QVariantList>::finished, this, [this, watcher, seq] {
        QVariantList results = watcher->result();
        watcher->deleteLater();
        if (seq == m_updatesSequence) {
            setBusy(false);
            emit updatesCompleted(results);
        }
    });

    QFuture<QVariantList> future = QtConcurrent::run([this] {
        return checkForUpdates();
    });
    watcher->setFuture(future);
}

void PackageManager::updateAllPackages() {
    setBusy(true);
    setStatusMessage(QStringLiteral("Updating all packages..."));
    appendLog(QStringLiteral("[%1] Starting full system & package updates...").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss"))));

    auto* watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher] {
        bool success = watcher->result();
        watcher->deleteLater();
        setBusy(false);
        setStatusMessage(QString());
        appendLog(QStringLiteral("[%1] %2: System update completed").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")), success ? QStringLiteral("SUCCESS") : QStringLiteral("FAILED")));
        emit operationFinished(success, success ? QStringLiteral("All packages updated successfully") : QStringLiteral("Some updates could not be applied"));
        checkForUpdatesAsync();
    });

    QFuture<bool> future = QtConcurrent::run([this] {
        return runSystemUpdate([this](const QString& line) { appendLog(line); });
    });
    watcher->setFuture(future);
}

bool PackageManager::runSystemUpdate(const std::function<void(const QString&)>& logCallback) {
    const auto stamped = [&logCallback](const QString& message) {
        logCallback(QStringLiteral("[%1] %2").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")), message));
    };

    bool success = true;

    if (enableFlatpak() && !QStandardPaths::findExecutable(QStringLiteral("flatpak")).isEmpty()) {
        stamped(QStringLiteral("Running Flatpak update (flatpak update -y)..."));
        success = astra::runProcessStreaming(QStringLiteral("flatpak"), {QStringLiteral("update"), QStringLiteral("-y")}, logCallback).succeeded() && success;
    }

    if (useCaelestiaUpdate() && !QStandardPaths::findExecutable(QStringLiteral("caelestia")).isEmpty()) {
        stamped(QStringLiteral("Running Caelestia update (caelestia update --noconfirm)..."));
        success = astra::runProcessStreaming(QStringLiteral("caelestia"), {QStringLiteral("update"), QStringLiteral("--noconfirm")}, logCallback).succeeded() && success;
    } else if (enablePacman() && !QStandardPaths::findExecutable(QStringLiteral("pacman")).isEmpty()) {
        stamped(QStringLiteral("Running Pacman update (pkexec pacman -Syu --noconfirm)..."));
        success = astra::runProcessStreaming(QStringLiteral("pkexec"), {QStringLiteral("pacman"), QStringLiteral("-Syu"), QStringLiteral("--noconfirm")}, logCallback).succeeded() && success;
    }

    return success;
}
