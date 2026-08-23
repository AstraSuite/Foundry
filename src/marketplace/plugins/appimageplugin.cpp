#include "appimageplugin.hpp"
#include "appimageinstaller.hpp"
#include "pluginprocess.hpp"

#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QUrl>

namespace {
constexpr int kCheckTimeoutMs = 30000;

QString updateTool() {
    return QStandardPaths::findExecutable(QStringLiteral("appimageupdatetool"));
}
}

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
    QVariantList updates;
    if (!m_enabled) return updates;

    const QString tool = updateTool();
    if (tool.isEmpty()) return updates;

    const QVariantList installed = getInstalled();
    for (const QVariant& value : installed) {
        const QVariantMap entry = value.toMap();
        const QString path = entry.value(QStringLiteral("path")).toString();
        if (path.isEmpty() || !QFile::exists(path)) continue;

        const astra::ProcessResult check = astra::runProcess(tool, {QStringLiteral("--check-for-update"), path}, kCheckTimeoutMs);
        if (!check.started || check.exitCode != 1) continue;

        QVariantMap item = entry;
        item[QStringLiteral("version")] = tr("new version available");
        updates.append(item);
    }
    return updates;
}

bool AppImagePlugin::update(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    Q_UNUSED(options);
    if (!m_enabled) return false;

    const QString tool = updateTool();
    if (tool.isEmpty()) {
        if (progressCb) progressCb(0, tr("appimageupdatetool is not installed, AppImages cannot be updated"));
        return false;
    }

    const QVariantMap entry = AppImageInstaller::findInstalledAppImage(packageId);
    const QString path = entry.value(QStringLiteral("path")).toString();
    if (path.isEmpty() || !QFile::exists(path)) {
        if (progressCb) progressCb(0, tr("No installed AppImage matches %1").arg(packageId));
        return false;
    }

    const astra::ProcessResult result = astra::runProcessStreaming(
        tool,
        {QStringLiteral("--overwrite"), path},
        [&progressCb](const QString& line) {
            if (progressCb) progressCb(50, line);
        },
        0, {}, m_cancellation);

    return result.succeeded();
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
