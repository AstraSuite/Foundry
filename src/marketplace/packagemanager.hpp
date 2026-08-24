#pragma once

#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QProcess>
#include <QQmlEngine>
#include <QThreadPool>
#include <QJSEngine>
#include <qqmlintegration.h>
#include <QList>
#include "plugins/packageplugin.hpp"
#include "plugins/flatpakplugin.hpp"
#include "plugins/pacmanplugin.hpp"
#include "plugins/aurplugin.hpp"
#include "plugins/appimageplugin.hpp"
#include "plugins/scriptableplugin.hpp"

class PackageManager : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool isBusy READ isBusy NOTIFY isBusyChanged)
    Q_PROPERTY(bool isFlatpakAvailable READ isFlatpakAvailable CONSTANT)
    Q_PROPERTY(bool isPacmanAvailable READ isPacmanAvailable CONSTANT)
    Q_PROPERTY(bool isAurAvailable READ isAurAvailable CONSTANT)
    Q_PROPERTY(bool isAppImageAvailable READ isAppImageAvailable CONSTANT)
    Q_PROPERTY(bool isCaelestiaAvailable READ isCaelestiaAvailable CONSTANT)
    Q_PROPERTY(bool useCaelestiaUpdate READ useCaelestiaUpdate WRITE setUseCaelestiaUpdate NOTIFY useCaelestiaUpdateChanged)
    Q_PROPERTY(bool enableFlatpak READ enableFlatpak WRITE setEnableFlatpak NOTIFY enableFlatpakChanged)
    Q_PROPERTY(bool enablePacman READ enablePacman WRITE setEnablePacman NOTIFY enablePacmanChanged)
    Q_PROPERTY(bool enableAur READ enableAur WRITE setEnableAur NOTIFY enableAurChanged)
    Q_PROPERTY(bool enableAppImage READ enableAppImage WRITE setEnableAppImage NOTIFY enableAppImageChanged)
    Q_PROPERTY(QString aurHelper READ aurHelper WRITE setAurHelper NOTIFY aurHelperChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QStringList logLines READ logLines NOTIFY logLinesChanged)
    Q_PROPERTY(int currentProgress READ currentProgress NOTIFY currentProgressChanged)
    Q_PROPERTY(QVariantList registeredPlugins READ getRegisteredPluginsInfo NOTIFY pluginsChanged)
    Q_PROPERTY(QStringList missingBackendTools READ missingBackendTools NOTIFY missingBackendToolsChanged)
    Q_PROPERTY(bool isCancellable READ isCancellable NOTIFY isCancellableChanged)
    Q_PROPERTY(bool isOperationRunning READ isOperationRunning NOTIFY isOperationRunningChanged)

public:
    explicit PackageManager(QObject* parent = nullptr);
    ~PackageManager() override;

    static PackageManager* create(QQmlEngine*, QJSEngine*);

    bool isBusy() const { return m_isBusy; }
    bool isCancellable() const { return m_cancellation != nullptr; }
    bool isOperationRunning() const { return m_operationRunning; }
    bool enableFlatpak() const;
    bool enablePacman() const;
    bool enableAur() const;
    bool enableAppImage() const;
    bool useCaelestiaUpdate() const;
    QString aurHelper() const;
    QString statusMessage() const { return m_statusMessage; }
    QStringList logLines() const { return m_logLines; }
    QStringList missingBackendTools() const;
    int currentProgress() const { return m_currentProgress; }

    bool isFlatpakAvailable() const;
    bool isPacmanAvailable() const;
    bool isAurAvailable() const;
    bool isAppImageAvailable() const;
    bool isCaelestiaAvailable() const;

    void setEnableFlatpak(bool enable);
    void setEnablePacman(bool enable);
    void setEnableAur(bool enable);
    void setEnableAppImage(bool enable);
    void setUseCaelestiaUpdate(bool enable);
    void setAurHelper(const QString& helper);

    bool runSystemUpdate(const std::function<void(const QString&)>& logCallback, const astra::CancellationTokenPtr& cancellation = {});

    void registerPlugin(IPackagePlugin* plugin);
    QList<IPackagePlugin*> plugins() const { return m_plugins; }
    IPackagePlugin* findPlugin(const QString& backendOrId) const;

    Q_INVOKABLE QVariantList getRegisteredPluginsInfo() const;
    Q_INVOKABLE QList<QVariantMap> getInstallSources(const QString& backend, const QString& packageId);
    Q_INVOKABLE QVariantList getInstalledPackages();
    Q_INVOKABLE bool isPackageInstalled(const QString& backend, const QString& packageId);
    Q_INVOKABLE QVariantList searchPackages(const QString& query, const QString& sourceFilter = QString());
    Q_INVOKABLE void searchPackagesAsync(const QString& query, const QString& sourceFilter = QString());
    Q_INVOKABLE void getInstalledPackagesAsync();
    Q_INVOKABLE void installPackage(const QString& backend, const QString& packageId, const QString& scope = QString());
    Q_INVOKABLE void updatePackage(const QString& backend, const QString& packageId, const QString& scope = QString());
    Q_INVOKABLE void uninstallPackage(const QString& backend, const QString& packageId, const QString& scope = QString());
    Q_INVOKABLE void launchApp(const QString& backend, const QString& packageId, const QString& execPath = QString());
    Q_INVOKABLE QString getIconPath(const QString& iconName, const QString& backend = QString());
    Q_INVOKABLE QVariantMap getPackageDetails(const QString& packageId, const QString& backend = QString());
    Q_INVOKABLE void fetchPackageDetailsAsync(const QString& packageId, const QString& backend = QString());
    Q_INVOKABLE void fetchCollectionAsync(const QString& collection, int limit = 12);
    Q_INVOKABLE void fetchBuildScriptAsync(const QString& packageId);
    Q_INVOKABLE QVariantList flatpakRemotes() const;
    Q_INVOKABLE void setFlatpakPermission(const QString& packageId, const QString& kind, const QString& value, const QString& access, bool enabled);
    Q_INVOKABLE void addFlatpakRemote(const QString& name, const QString& url, const QString& scope = QStringLiteral("user"));
    Q_INVOKABLE void removeFlatpakRemote(const QString& name, const QString& scope = QStringLiteral("user"));
    Q_INVOKABLE QVariantList checkForUpdates();
    Q_INVOKABLE void checkForUpdatesAsync();
    Q_INVOKABLE void updateAllPackages();
    Q_INVOKABLE QVariantList recentOperations(int limit = 8) const;
    void recordOperation(const QString& action, const QString& packageId, const QString& backend, bool success);
    Q_INVOKABLE void cancelCurrentOperation();
    Q_INVOKABLE void appendLog(const QString& line);
    Q_INVOKABLE void clearLogs();

signals:
    void isBusyChanged();
    void isCancellableChanged();
    void isOperationRunningChanged();
    void enableFlatpakChanged();
    void enablePacmanChanged();
    void enableAurChanged();
    void enableAppImageChanged();
    void useCaelestiaUpdateChanged();
    void aurHelperChanged();
    void statusMessageChanged();
    void logLinesChanged();
    void currentProgressChanged();
    void pluginsChanged();
    void historyChanged();
    void missingBackendToolsChanged();
    void operationProgress(const QString& packageId, int percent, const QString& status);
    void operationFinished(bool success, const QString& message);
    void searchCompleted(const QVariantList& results);
    void installedCompleted(const QVariantList& results);
    void updatesCompleted(const QVariantList& updates);
    void packageDetailsReady(const QVariantMap& details);
    void collectionReady(const QString& collection, const QVariantList& apps);
    void buildScriptReady(const QString& packageId, const QString& script);
    void remotesChanged();
    void remoteOperationFinished(bool success, const QString& message);

private:
    void setBusy(bool busy);
    void setOperationRunning(bool running);
    astra::CancellationTokenPtr beginCancellableOperation(IPackagePlugin* plugin);
    void endCancellableOperation();
    void setStatusMessage(const QString& message);
    void initPlugins();

    bool m_isBusy{false};
    bool m_operationRunning{false};
    QString m_statusMessage;
    QStringList m_logLines;
    int m_currentProgress{0};

    FlatpakPlugin* m_flatpakPlugin{nullptr};
    PacmanPlugin* m_pacmanPlugin{nullptr};
    AurPlugin* m_aurPlugin{nullptr};
    AppImagePlugin* m_appimagePlugin{nullptr};
    QList<IPackagePlugin*> m_plugins;

    QThreadPool m_pluginPool;
    astra::CancellationTokenPtr m_cancellation;
    bool m_lastOperationCancelled{false};
    QHash<QString, QVariantList> m_collections;
    QHash<QString, QString> m_buildScripts;
    QSet<QString> m_pendingCollections;

    quint64 m_searchSequence{0};
    quint64 m_installedSequence{0};
    quint64 m_updatesSequence{0};
};

