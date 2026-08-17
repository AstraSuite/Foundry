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

    QSet<QString> seenIds;
    QString iconsDir = QDir::homePath() + QStringLiteral("/.local/share/icons/hicolor/512x512/apps/");

    // 1. Scan registered desktop files in ~/.local/share/applications
    QString appsDir = QDir::homePath() + QStringLiteral("/.local/share/applications");
    QDir dir(appsDir);
    if (dir.exists()) {
        QStringList desktopFiles = dir.entryList({QStringLiteral("appimage-*.desktop")}, QDir::Files);
        for (const QString& dFile : desktopFiles) {
            QFile file(appsDir + QStringLiteral("/") + dFile);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QVariantMap item;
                item[QStringLiteral("backend")] = QStringLiteral("AppImage");
                item[QStringLiteral("scope")] = QStringLiteral("user");
                item[QStringLiteral("version")] = QStringLiteral("portable");
                item[QStringLiteral("isInstalled")] = true;

                QString baseId = dFile.mid(9); // strip "appimage-"
                if (baseId.endsWith(QLatin1String(".desktop"))) {
                    baseId.chop(8);
                }
                item[QStringLiteral("id")] = baseId;

                QTextStream stream(&file);
                while (!stream.atEnd()) {
                    QString line = stream.readLine().trimmed();
                    if (line.startsWith(QLatin1String("Name="))) {
                        item[QStringLiteral("name")] = line.mid(5).trimmed();
                    } else if (line.startsWith(QLatin1String("Exec="))) {
                        QString exec = line.mid(5).trimmed();
                        exec.remove(QLatin1Char('"'));
                        exec.remove(QStringLiteral(" %U"));
                        item[QStringLiteral("path")] = exec;
                    } else if (line.startsWith(QLatin1String("Icon="))) {
                        item[QStringLiteral("icon")] = line.mid(5).trimmed();
                    } else if (line.startsWith(QLatin1String("Comment="))) {
                        item[QStringLiteral("summary")] = line.mid(8).trimmed();
                    }
                }

                if (item.value(QStringLiteral("name")).toString().isEmpty()) {
                    item[QStringLiteral("name")] = baseId;
                }

                QString icon = item.value(QStringLiteral("icon")).toString();
                if (!icon.isEmpty() && !QFile::exists(icon)) {
                    if (QFile::exists(iconsDir + icon)) {
                        item[QStringLiteral("icon")] = iconsDir + icon;
                    } else if (QFile::exists(iconsDir + icon + QStringLiteral(".png"))) {
                        item[QStringLiteral("icon")] = iconsDir + icon + QStringLiteral(".png");
                    } else if (QFile::exists(iconsDir + icon + QStringLiteral(".svg"))) {
                        item[QStringLiteral("icon")] = iconsDir + icon + QStringLiteral(".svg");
                    }
                }

                seenIds.insert(baseId);
                results.append(item);
            }
        }
    }

    // 2. Also check ~/Applications for any standalone AppImages not registered via desktop
    QString standAloneDir = QDir::homePath() + QStringLiteral("/Applications");
    QDir saDir(standAloneDir);
    if (saDir.exists()) {
        QStringList files = saDir.entryList({QStringLiteral("*.AppImage"), QStringLiteral("*.appimage")}, QDir::Files);
        for (const QString& file : files) {
            QFileInfo fi(saDir.filePath(file));
            QString id = fi.baseName().toLower();
            if (seenIds.contains(id)) continue;

            QVariantMap item;
            item[QStringLiteral("id")] = fi.baseName();
            item[QStringLiteral("name")] = fi.baseName();
            item[QStringLiteral("version")] = QStringLiteral("portable");
            item[QStringLiteral("size")] = QString::number(fi.size() / (1024 * 1024)) + QStringLiteral(" MB");
            item[QStringLiteral("backend")] = QStringLiteral("AppImage");
            item[QStringLiteral("scope")] = QStringLiteral("user");
            item[QStringLiteral("path")] = fi.absoluteFilePath();
            item[QStringLiteral("isInstalled")] = true;

            QString icon = fi.baseName();
            if (QFile::exists(iconsDir + icon + QStringLiteral(".png"))) {
                item[QStringLiteral("icon")] = iconsDir + icon + QStringLiteral(".png");
            } else if (QFile::exists(iconsDir + icon + QStringLiteral(".svg"))) {
                item[QStringLiteral("icon")] = iconsDir + icon + QStringLiteral(".svg");
            } else {
                item[QStringLiteral("icon")] = icon;
            }

            seenIds.insert(id);
            results.append(item);
        }
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
