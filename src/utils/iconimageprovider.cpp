#include "iconimageprovider.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QMutexLocker>
#include <QPainter>
#include <QSvgRenderer>
#include <QTextStream>
#include <QUrl>

namespace {

const QStringList& iconSubDirectories() {
    static const QStringList subDirectories{
        QStringLiteral("scalable/apps"),
        QStringLiteral("512x512/apps"),
        QStringLiteral("256x256/apps"),
        QStringLiteral("128x128/apps"),
        QStringLiteral("64x64/apps"),
        QStringLiteral("48x48/apps"),
        QStringLiteral("32x32/apps"),
        QStringLiteral("symbolic/apps"),
        QStringLiteral("apps"),
        QStringLiteral("scalable/categories"),
        QStringLiteral("128x128/categories"),
        QStringLiteral("64x64/categories"),
        QStringLiteral("48x48/categories"),
        QString()
    };
    return subDirectories;
}

const QStringList& iconExtensions() {
    static const QStringList extensions{
        QStringLiteral(".svg"),
        QStringLiteral(".png"),
        QStringLiteral(".xpm")
    };
    return extensions;
}

QStringList buildThemeDirectories() {
    QString theme = QIcon::themeName();
    if (theme.isEmpty() || theme == QStringLiteral("hicolor")) theme = QStringLiteral("Papirus-Dark");

    QStringList themeNames{theme};
    for (const QString& fallback : {QStringLiteral("Papirus-Dark"), QStringLiteral("Papirus"), QStringLiteral("breeze-dark"), QStringLiteral("breeze"), QStringLiteral("Adwaita")}) {
        if (!themeNames.contains(fallback)) themeNames.append(fallback);
    }

    const QStringList roots{
        QDir::homePath() + QStringLiteral("/.local/share/icons"),
        QStringLiteral("/usr/share/icons")
    };

    QStringList directories;
    for (const QString& themeName : themeNames) {
        for (const QString& root : roots) {
            const QString path = root + QStringLiteral("/") + themeName;
            if (QFileInfo::exists(path)) directories.append(path);
        }
    }

    const QStringList extra{
        QDir::homePath() + QStringLiteral("/.local/share/flatpak/exports/share/icons/hicolor"),
        QStringLiteral("/var/lib/flatpak/exports/share/icons/hicolor"),
        QDir::homePath() + QStringLiteral("/.local/share/icons/hicolor"),
        QStringLiteral("/usr/share/icons/hicolor"),
        QDir::homePath() + QStringLiteral("/.local/share/icons"),
        QStringLiteral("/usr/share/pixmaps")
    };
    for (const QString& path : extra) {
        if (QFileInfo::exists(path) && !directories.contains(path)) directories.append(path);
    }

    return directories;
}

const QStringList& desktopEntryDirectories() {
    static const QStringList directories{
        QStringLiteral("/usr/share/applications"),
        QDir::homePath() + QStringLiteral("/.local/share/applications"),
        QDir::homePath() + QStringLiteral("/.local/share/flatpak/exports/share/applications"),
        QStringLiteral("/var/lib/flatpak/exports/share/applications")
    };
    return directories;
}

QImage loadImage(const QString& path, const QSize& targetSize) {
    if (path.isEmpty()) return QImage();

    if (path.endsWith(QLatin1String(".svg"), Qt::CaseInsensitive)) {
        QSvgRenderer renderer(path);
        if (!renderer.isValid()) return QImage();

        QImage image(targetSize, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        renderer.render(&painter);
        return image;
    }

    return QImage(path);
}

}

IconImageProvider::IconImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
    , m_themeDirectories(buildThemeDirectories()) {}

QImage IconImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize) {
    QString name = id;
    if (name.contains(QLatin1Char('?'))) name = name.section(QLatin1Char('?'), 0, 0);
    if (name.startsWith(QLatin1String("image://icon/"))) name = name.mid(13);
    if (name.startsWith(QLatin1String("file://"))) name = QUrl(name).toLocalFile();
    if (name.isEmpty()) return QImage();

    const QSize targetSize(requestedSize.width() > 0 ? requestedSize.width() : 128,
                           requestedSize.height() > 0 ? requestedSize.height() : 128);

    const QImage image = loadImage(resolvePath(name), targetSize);
    if (!image.isNull() && size) *size = image.size();
    return image;
}

QString IconImageProvider::resolvePath(const QString& name) {
    {
        QMutexLocker locker(&m_mutex);
        const auto cached = m_resolvedPaths.constFind(name);
        if (cached != m_resolvedPaths.constEnd()) return cached.value();
    }

    QString path;
    if (name.startsWith(QLatin1Char('/'))) {
        if (QFile::exists(name)) path = name;
    } else {
        path = searchThemeDirectories(name);
        if (path.isEmpty()) path = searchDesktopEntries(name);
    }

    QMutexLocker locker(&m_mutex);
    m_resolvedPaths.insert(name, path);
    return path;
}

QString IconImageProvider::searchThemeDirectories(const QString& name) const {
    QStringList candidates{name};
    const QString lowered = name.toLower();
    if (lowered != name) candidates.append(lowered);
    if (name.contains(QLatin1Char('.'))) candidates.append(name.section(QLatin1Char('.'), -1));

    for (const QString& candidate : candidates) {
        if (candidate.isEmpty()) continue;
        for (const QString& directory : m_themeDirectories) {
            for (const QString& subDirectory : iconSubDirectories()) {
                const QString base = subDirectory.isEmpty()
                    ? directory + QStringLiteral("/") + candidate
                    : directory + QStringLiteral("/") + subDirectory + QStringLiteral("/") + candidate;

                for (const QString& extension : iconExtensions()) {
                    const QString path = base + extension;
                    if (QFile::exists(path)) return path;
                }
            }
        }
    }
    return QString();
}

void IconImageProvider::loadDesktopEntries() {
    if (m_desktopEntriesLoaded) return;
    m_desktopEntriesLoaded = true;

    for (const QString& directoryPath : desktopEntryDirectories()) {
        QDir directory(directoryPath);
        if (!directory.exists()) continue;

        const QStringList entries = directory.entryList({QStringLiteral("*.desktop")}, QDir::Files);
        for (const QString& entry : entries) {
            QFile file(directory.filePath(entry));
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

            QString icon;
            QTextStream stream(&file);
            while (!stream.atEnd()) {
                const QString line = stream.readLine().trimmed();
                if (line.startsWith(QLatin1String("Icon="))) {
                    icon = line.mid(5).trimmed();
                    break;
                }
            }
            if (icon.isEmpty()) continue;

            QString key = entry;
            key.chop(8);
            m_desktopIcons.insert(key.toLower(), icon);
        }
    }
}

QString IconImageProvider::searchDesktopEntries(const QString& name) {
    QMutexLocker locker(&m_mutex);
    loadDesktopEntries();

    const QString lowered = name.toLower();
    QString icon = m_desktopIcons.value(lowered);

    if (icon.isEmpty()) {
        for (auto it = m_desktopIcons.constBegin(); it != m_desktopIcons.constEnd(); ++it) {
            if (it.key().contains(lowered)) {
                icon = it.value();
                break;
            }
        }
    }
    locker.unlock();

    if (icon.isEmpty()) return QString();
    if (icon.startsWith(QLatin1Char('/'))) return QFile::exists(icon) ? icon : QString();
    return searchThemeDirectories(icon);
}
