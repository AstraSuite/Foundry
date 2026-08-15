#include "appimageplugin.hpp"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTextStream>

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

    QVariantList installed = getInstalled();
    QString q = query.trimmed().toLower();
    for (const QVariant& v : installed) {
        QVariantMap map = v.toMap();
        if (map.value(QStringLiteral("name")).toString().toLower().contains(q) ||
            map.value(QStringLiteral("id")).toString().toLower().contains(q)) {
            results.append(map);
        }
    }
    return results;
}

QVariantList AppImagePlugin::getInstalled() {
    QVariantList results;
    if (!m_enabled) return results;

    QString appDir = QDir::homePath() + QStringLiteral("/Applications");
    QDir dir(appDir);
    if (!dir.exists()) return results;

    QStringList files = dir.entryList({QStringLiteral("*.AppImage"), QStringLiteral("*.appimage")}, QDir::Files);
    for (const QString& file : files) {
        QFileInfo fi(dir.filePath(file));
        QVariantMap item;
        item[QStringLiteral("id")] = fi.baseName();
        item[QStringLiteral("name")] = fi.baseName();
        item[QStringLiteral("version")] = QStringLiteral("portable");
        item[QStringLiteral("size")] = QString::number(fi.size() / (1024 * 1024)) + QStringLiteral(" MB");
        item[QStringLiteral("backend")] = QStringLiteral("AppImage");
        item[QStringLiteral("scope")] = QStringLiteral("user");
        item[QStringLiteral("path")] = fi.absoluteFilePath();
        item[QStringLiteral("icon")] = fi.baseName();
        item[QStringLiteral("isInstalled")] = true;
        results.append(item);
    }
    return results;
}

QVariantList AppImagePlugin::getUpdates() {
    return {};
}

QVariantMap AppImagePlugin::getDetails(const QString& packageId) {
    QVariantMap map;
    map[QStringLiteral("id")] = packageId;
    map[QStringLiteral("name")] = packageId;
    map[QStringLiteral("backend")] = QStringLiteral("AppImage");
    map[QStringLiteral("summary")] = QStringLiteral("Standalone AppImage application");
    map[QStringLiteral("description")] = QStringLiteral("Standalone AppImage application");
    return map;
}

bool AppImagePlugin::install(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    Q_UNUSED(packageId);
    Q_UNUSED(options);
    Q_UNUSED(progressCb);
    return false;
}

bool AppImagePlugin::uninstall(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    Q_UNUSED(options);
    Q_UNUSED(progressCb);
    QString appDir = QDir::homePath() + QStringLiteral("/Applications");
    QDir dir(appDir);
    QStringList files = dir.entryList({packageId + QStringLiteral("*.AppImage"), packageId + QStringLiteral("*.appimage")}, QDir::Files);
    for (const QString& file : files) {
        QFile::remove(dir.filePath(file));
    }
    return true;
}

bool AppImagePlugin::launch(const QString& packageId) {
    QString appDir = QDir::homePath() + QStringLiteral("/Applications");
    QDir dir(appDir);
    QStringList files = dir.entryList({packageId + QStringLiteral("*.AppImage"), packageId + QStringLiteral("*.appimage")}, QDir::Files);
    if (!files.isEmpty()) {
        return QProcess::startDetached(dir.filePath(files.first()));
    }
    return false;
}
