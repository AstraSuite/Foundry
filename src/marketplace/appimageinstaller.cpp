#include "appimageinstaller.hpp"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSet>
#include <QUrl>
#include <QStandardPaths>
#include <QDebug>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTextStream>

namespace {

QString applicationsDirectory() {
    return QDir::homePath() + QStringLiteral("/.local/share/applications");
}

QString binaryDirectory() {
    return QDir::homePath() + QStringLiteral("/.local/bin");
}

QString iconDirectory() {
    return QDir::homePath() + QStringLiteral("/.local/share/icons/hicolor/512x512/apps");
}

QString portableDirectory() {
    return QDir::homePath() + QStringLiteral("/Applications");
}

QString slugify(const QString& value) {
    static const QRegularExpression unsupported(QStringLiteral("[^a-z0-9_-]"));
    QString slug = value.toLower();
    slug.replace(unsupported, QStringLiteral("-"));
    return slug;
}

QString resolveIcon(const QString& icon) {
    if (icon.isEmpty()) return QString();
    if (icon.startsWith(QLatin1Char('/'))) return QFile::exists(icon) ? icon : QString();

    const QString base = iconDirectory() + QStringLiteral("/") + icon;
    for (const QString& candidate : {base, base + QStringLiteral(".png"), base + QStringLiteral(".svg")}) {
        if (QFile::exists(candidate)) return candidate;
    }
    return icon;
}

QString executableFromExecLine(QString exec) {
    exec.remove(QLatin1Char('"'));
    exec.remove(QStringLiteral(" %U"));
    exec.remove(QStringLiteral(" %u"));
    exec.remove(QStringLiteral(" %F"));
    exec.remove(QStringLiteral(" %f"));
    return exec.trimmed();
}

QString humanSize(qint64 bytes) {
    if (bytes <= 0) return QString();
    return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + QStringLiteral(" MB");
}

}

AppImageInstaller::AppImageInstaller(QObject* parent)
    : QObject(parent) {}

AppImageInstaller* AppImageInstaller::create(QQmlEngine*, QJSEngine*) {
    return new AppImageInstaller();
}

void AppImageInstaller::setStatus(bool installing, const QString& message) {
    m_isInstalling = installing;
    m_statusMessage = message;
    emit isInstallingChanged();
    emit statusMessageChanged();
}

bool AppImageInstaller::installAppImage(const QString& fileUrlOrPath) {
    QString localPath = fileUrlOrPath;
    if (localPath.startsWith(QLatin1String("file://"))) {
        localPath = QUrl(fileUrlOrPath).toLocalFile();
    }

    QFileInfo fileInfo(localPath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        setStatus(false, QStringLiteral("File does not exist: ") + localPath);
        emit appImageInstallationFailed(m_statusMessage);
        return false;
    }

    QString appBaseName = fileInfo.completeBaseName();

    const QString safeAppName = slugify(appBaseName);

    setStatus(true, QStringLiteral("Installing ") + appBaseName + QStringLiteral("..."));

    const QString binDir = binaryDirectory();
    QDir().mkpath(binDir);

    QString destAppImagePath = binDir + QStringLiteral("/") + safeAppName + QStringLiteral(".AppImage");

    if (QFile::exists(destAppImagePath)) {
        QFile::remove(destAppImagePath);
    }

    if (!QFile::copy(localPath, destAppImagePath)) {
        setStatus(false, QStringLiteral("Failed to copy AppImage to ") + destAppImagePath);
        emit appImageInstallationFailed(m_statusMessage);
        return false;
    }

    QFile::setPermissions(destAppImagePath,
        QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser |
        QFileDevice::ReadGroup | QFileDevice::ExeGroup |
        QFileDevice::ReadOther | QFileDevice::ExeOther);

    QTemporaryDir tempDir;
    QString iconPath;
    QString displayName = appBaseName;

    if (tempDir.isValid()) {
        QProcess extractProc;
        extractProc.setWorkingDirectory(tempDir.path());
        extractProc.start(destAppImagePath, {QStringLiteral("--appimage-extract")});
        if (extractProc.waitForFinished(10000)) {
            QString squashDir = tempDir.path() + QStringLiteral("/squashfs-root");
            if (QDir(squashDir).exists()) {
                iconPath = extractIcon(squashDir, safeAppName);

                QDir squashQDir(squashDir);
                QStringList desktopFiles = squashQDir.entryList({QStringLiteral("*.desktop")}, QDir::Files);
                if (!desktopFiles.isEmpty()) {
                    QFile dFile(squashDir + QStringLiteral("/") + desktopFiles.first());
                    if (dFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                        QTextStream stream(&dFile);
                        while (!stream.atEnd()) {
                            QString line = stream.readLine().trimmed();
                            if (line.startsWith(QLatin1String("Name="))) {
                                displayName = line.mid(5).trimmed();
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    QString desktopPath = generateDesktopFile(safeAppName, destAppImagePath, iconPath, displayName);

    QProcess::startDetached(QStringLiteral("update-desktop-database"), {applicationsDirectory()});

    if (QFile::exists(QStringLiteral("/usr/bin/notify-send")) || QFile::exists(QStringLiteral("/bin/notify-send"))) {
        QString notifIcon = iconPath.isEmpty() ? QStringLiteral("system-software-install") : iconPath;
        QProcess::startDetached(QStringLiteral("notify-send"), {
            QStringLiteral("Foundry"),
            QStringLiteral("Successfully installed ") + displayName + QStringLiteral("\nAvailable in your applications menu."),
            QStringLiteral("-i"), notifIcon
        });
    }

    setStatus(false, QStringLiteral("Successfully installed ") + displayName);
    emit appImageInstalled(displayName, desktopPath);
    return true;
}

QString AppImageInstaller::extractIcon(const QString& tempExtractDir, const QString& appName) {
    QDir extractDir(tempExtractDir);
    const QString iconsDestDir = iconDirectory();
    QDir().mkpath(iconsDestDir);

    QStringList iconFilters;
    iconFilters << QStringLiteral("*.png") << QStringLiteral("*.svg") << QStringLiteral("*.xpm");

    QStringList rootIcons = extractDir.entryList(iconFilters, QDir::Files);
    QString foundIcon;

    if (!rootIcons.isEmpty()) {
        foundIcon = tempExtractDir + QStringLiteral("/") + rootIcons.first();
    } else {

        if (QFile::exists(tempExtractDir + QStringLiteral("/.DirIcon"))) {
            foundIcon = tempExtractDir + QStringLiteral("/.DirIcon");
        }
    }

    if (!foundIcon.isEmpty() && QFile::exists(foundIcon)) {
        QFileInfo iconInfo(foundIcon);
        QString ext = iconInfo.suffix().isEmpty() ? QStringLiteral("png") : iconInfo.suffix();
        QString destIconPath = iconsDestDir + QStringLiteral("/") + appName + QStringLiteral(".") + ext;

        if (QFile::exists(destIconPath)) {
            QFile::remove(destIconPath);
        }

        if (QFile::copy(foundIcon, destIconPath)) {
            return destIconPath;
        }
    }

    return appName;
}

QString AppImageInstaller::generateDesktopFile(const QString& appName, const QString& execPath, const QString& iconPath, const QString& displayName) {
    const QString appsDir = applicationsDirectory();
    QDir().mkpath(appsDir);

    QString desktopPath = appsDir + QStringLiteral("/appimage-") + appName + QStringLiteral(".desktop");

    QFile file(desktopPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "[Desktop Entry]\n"
            << "Type=Application\n"
            << "Name=" << displayName << "\n"
            << "Exec=\"" << execPath << "\" %U\n"
            << "Icon=" << (iconPath.isEmpty() ? appName : iconPath) << "\n"
            << "Terminal=false\n"
            << "Categories=Utility;\n"
            << "Comment=Installed via Foundry\n"
            << "X-AppImage-InstalledBy=astra-foundry\n";
        file.close();
    }

    return desktopPath;
}

QVariantList AppImageInstaller::installedAppImages() {
    QVariantList entries;
    QSet<QString> seenPaths;

    QDir applications(applicationsDirectory());
    const QStringList desktopFiles = applications.entryList({QStringLiteral("appimage-*.desktop")}, QDir::Files, QDir::Name);
    for (const QString& fileName : desktopFiles) {
        QFile file(applications.filePath(fileName));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

        QString identifier = fileName.mid(9);
        if (identifier.endsWith(QLatin1String(".desktop"))) identifier.chop(8);

        QString name;
        QString exec;
        QString icon;
        QString comment;

        QTextStream stream(&file);
        while (!stream.atEnd()) {
            const QString line = stream.readLine().trimmed();
            if (line.startsWith(QLatin1String("Name="))) name = line.mid(5).trimmed();
            else if (line.startsWith(QLatin1String("Exec="))) exec = executableFromExecLine(line.mid(5).trimmed());
            else if (line.startsWith(QLatin1String("Icon="))) icon = line.mid(5).trimmed();
            else if (line.startsWith(QLatin1String("Comment="))) comment = line.mid(8).trimmed();
        }

        const QFileInfo executable(exec);

        QVariantMap entry;
        entry[QStringLiteral("id")] = identifier;
        entry[QStringLiteral("name")] = name.isEmpty() ? identifier : name;
        entry[QStringLiteral("exec")] = exec;
        entry[QStringLiteral("path")] = exec;
        entry[QStringLiteral("icon")] = resolveIcon(icon.isEmpty() ? identifier : icon);
        entry[QStringLiteral("desktopFile")] = fileName;
        entry[QStringLiteral("desktopPath")] = applications.filePath(fileName);
        entry[QStringLiteral("summary")] = comment;
        entry[QStringLiteral("size")] = executable.exists() ? humanSize(executable.size()) : QString();
        entries.append(entry);

        if (executable.exists()) seenPaths.insert(executable.absoluteFilePath());
    }

    for (const QString& directory : {portableDirectory(), binaryDirectory()}) {
        QDir dir(directory);
        if (!dir.exists()) continue;

        const QStringList files = dir.entryList({QStringLiteral("*.AppImage"), QStringLiteral("*.appimage")}, QDir::Files, QDir::Name);
        for (const QString& fileName : files) {
            const QFileInfo info(dir.filePath(fileName));
            if (seenPaths.contains(info.absoluteFilePath())) continue;
            seenPaths.insert(info.absoluteFilePath());

            QVariantMap entry;
            entry[QStringLiteral("id")] = info.completeBaseName();
            entry[QStringLiteral("name")] = info.completeBaseName();
            entry[QStringLiteral("exec")] = info.absoluteFilePath();
            entry[QStringLiteral("path")] = info.absoluteFilePath();
            entry[QStringLiteral("icon")] = resolveIcon(slugify(info.completeBaseName()));
            entry[QStringLiteral("summary")] = QString();
            entry[QStringLiteral("size")] = humanSize(info.size());
            entries.append(entry);
        }
    }

    return entries;
}

QVariantMap AppImageInstaller::findInstalledAppImage(const QString& appIdentifier) {
    if (appIdentifier.isEmpty()) return {};

    QString identifier = appIdentifier;
    if (identifier.startsWith(QLatin1String("file://"))) identifier = QUrl(identifier).toLocalFile();
    const QString slug = slugify(identifier);

    const QVariantList entries = installedAppImages();
    const QStringList keys{QStringLiteral("desktopPath"), QStringLiteral("path"), QStringLiteral("id"), QStringLiteral("name")};
    for (const QString& key : keys) {
        for (const QVariant& value : entries) {
            const QVariantMap entry = value.toMap();
            if (entry.value(key).toString().compare(identifier, Qt::CaseInsensitive) == 0) return entry;
        }
    }

    for (const QVariant& value : entries) {
        const QVariantMap entry = value.toMap();
        if (slugify(entry.value(QStringLiteral("id")).toString()) == slug) return entry;
        if (slugify(entry.value(QStringLiteral("name")).toString()) == slug) return entry;
    }

    return {};
}

QVariantList AppImageInstaller::listInstalledAppImages() {
    return installedAppImages();
}

bool AppImageInstaller::uninstallAppImage(const QString& appIdentifier) {
    const QVariantMap entry = findInstalledAppImage(appIdentifier);
    if (entry.isEmpty()) return false;

    bool removed = false;

    const QString executable = entry.value(QStringLiteral("path")).toString();
    if (!executable.isEmpty() && QFile::exists(executable)) removed |= QFile::remove(executable);

    const QString desktopPath = entry.value(QStringLiteral("desktopPath")).toString();
    if (!desktopPath.isEmpty() && QFile::exists(desktopPath)) removed |= QFile::remove(desktopPath);

    const QString icon = entry.value(QStringLiteral("icon")).toString();
    if (icon.startsWith(iconDirectory()) && QFile::exists(icon)) QFile::remove(icon);

    if (!desktopPath.isEmpty()) {
        QProcess::startDetached(QStringLiteral("update-desktop-database"), {applicationsDirectory()});
    }

    return removed;
}

bool AppImageInstaller::launchAppImage(const QString& appIdentifier) {
    const QVariantMap entry = findInstalledAppImage(appIdentifier);
    const QString executable = entry.value(QStringLiteral("path")).toString();
    if (executable.isEmpty() || !QFile::exists(executable)) return false;

    return QProcess::startDetached(executable, {});
}
