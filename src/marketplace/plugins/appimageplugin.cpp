#include "appimageplugin.hpp"
#include "appimageinstaller.hpp"

#include <QFileInfo>
#include <QProcess>
#include <QUrl>

AppImagePlugin::AppImagePlugin(QObject* parent) : IPackagePlugin(parent) {
    m_enabled = m_settings.value(QStringLiteral("AppImage/enabled"), true).toBool();
}

void AppImagePlugin::setEnabled(bool enabled) {
    if (m_enabled != enabled) {
        m_enabled = enabled;
        m_settings.setValue(QStringLiteral("AppImage/enabled"), enabled);
        emit enabledChanged();
    }
}

QVariantList AppImagePlugin::search(const QString& query, const QVariantMap& options) {
    Q_UNUSED(options);
    QVariantList results;
    if (!m_enabled) return results;

    const QString needle = query.trimmed().toLower();
    const QVariantList installed = getInstalled();
    for (const QVariant& value : installed) {
        const QVariantMap item = value.toMap();
        if (item.value(QStringLiteral("name")).toString().toLower().contains(needle) ||
            item.value(QStringLiteral("id")).toString().toLower().contains(needle)) {
            results.append(item);
        }
    }
    return results;
}

QVariantList AppImagePlugin::getInstalled() {
    QVariantList results;
    if (!m_enabled) return results;

    const QVariantList entries = AppImageInstaller::installedAppImages();
    for (const QVariant& value : entries) {
        QVariantMap item = value.toMap();
        item[QStringLiteral("backend")] = QStringLiteral("AppImage");
        item[QStringLiteral("scope")] = QStringLiteral("user");
        item[QStringLiteral("version")] = QStringLiteral("portable");
        item[QStringLiteral("isInstalled")] = true;
        if (item.value(QStringLiteral("icon")).toString().isEmpty()) {
            item[QStringLiteral("icon")] = item.value(QStringLiteral("id"));
        }
        results.append(item);
    }
    return results;
}

QVariantList AppImagePlugin::getUpdates() {
    return {};
}

QVariantMap AppImagePlugin::getDetails(const QString& packageId) {
    QVariantMap map = AppImageInstaller::findInstalledAppImage(packageId);
    map[QStringLiteral("id")] = map.value(QStringLiteral("id"), packageId);
    map[QStringLiteral("name")] = map.value(QStringLiteral("name"), packageId);
    map[QStringLiteral("backend")] = QStringLiteral("AppImage");
    map[QStringLiteral("version")] = QStringLiteral("portable");

    const QString summary = map.value(QStringLiteral("summary")).toString();
    if (summary.isEmpty()) {
        map[QStringLiteral("summary")] = QStringLiteral("Standalone AppImage application");
    }
    map[QStringLiteral("description")] = map.value(QStringLiteral("path")).toString().isEmpty()
        ? map.value(QStringLiteral("summary"))
        : map.value(QStringLiteral("path"));

    return map;
}

bool AppImagePlugin::install(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    Q_UNUSED(options);
    if (!m_enabled) return false;

    QString path = packageId;
    if (path.startsWith(QLatin1String("file://"))) path = QUrl(path).toLocalFile();

    const QFileInfo info(path);
    if (!info.exists() || !info.isFile()) {
        if (progressCb) progressCb(0, QStringLiteral("AppImage file not found: ") + path);
        return false;
    }

    if (progressCb) progressCb(10, QStringLiteral("Installing ") + info.fileName());

    AppImageInstaller installer;
    const bool installed = installer.installAppImage(info.absoluteFilePath());

    if (progressCb) progressCb(installed ? 100 : 0, installer.statusMessage());
    return installed;
}

bool AppImagePlugin::uninstall(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    Q_UNUSED(options);

    AppImageInstaller installer;
    const bool removed = installer.uninstallAppImage(packageId);

    if (progressCb) {
        progressCb(removed ? 100 : 0, removed
            ? QStringLiteral("Removed ") + packageId
            : QStringLiteral("No installed AppImage matches ") + packageId);
    }
    return removed;
}

bool AppImagePlugin::launch(const QString& packageId) {
    AppImageInstaller installer;
    return installer.launchAppImage(packageId);
}
