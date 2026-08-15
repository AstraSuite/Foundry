#include "appimageinstaller.hpp"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QStandardPaths>
#include <QDebug>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTextStream>

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

    QString safeAppName = appBaseName.toLower();
    safeAppName.replace(QRegularExpression(QStringLiteral("[^a-z0-9_-]")), QStringLiteral("-"));

    setStatus(true, QStringLiteral("Installing ") + appBaseName + QStringLiteral("..."));

    QString binDir = QDir::homePath() + QStringLiteral("/.local/bin");
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

    QProcess::startDetached(QStringLiteral("update-desktop-database"), {QDir::homePath() + QStringLiteral("/.local/share/applications")});

    setStatus(false, QStringLiteral("Successfully installed ") + displayName);
    emit appImageInstalled(displayName, desktopPath);
    return true;
}

QString AppImageInstaller::extractIcon(const QString& tempExtractDir, const QString& appName) {
    QDir extractDir(tempExtractDir);
    QString iconsDestDir = QDir::homePath() + QStringLiteral("/.local/share/icons/hicolor/512x512/apps");
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
    QString appsDir = QDir::homePath() + QStringLiteral("/.local/share/applications");
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
            << "Comment=Installed via AstraMarket\n"
            << "X-AppImage-InstalledBy=AstraMarket\n";
        file.close();
    }

    return desktopPath;
}

QVariantList AppImageInstaller::listInstalledAppImages() {
    QVariantList list;
    QString appsDir = QDir::homePath() + QStringLiteral("/.local/share/applications");
    QDir dir(appsDir);

    QStringList desktopFiles = dir.entryList({QStringLiteral("appimage-*.desktop")}, QDir::Files);
    for (const QString& dFile : desktopFiles) {
        QFile file(appsDir + QStringLiteral("/") + dFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QVariantMap appMap;
            appMap[QStringLiteral("desktopFile")] = dFile;
            appMap[QStringLiteral("desktopPath")] = appsDir + QStringLiteral("/") + dFile;

            QTextStream stream(&file);
            while (!stream.atEnd()) {
                QString line = stream.readLine().trimmed();
                if (line.startsWith(QLatin1String("Name="))) {
                    appMap[QStringLiteral("name")] = line.mid(5).trimmed();
                } else if (line.startsWith(QLatin1String("Exec="))) {
                    QString exec = line.mid(5).trimmed();
                    exec.remove(QLatin1Char('"'));
                    exec.remove(QStringLiteral(" %U"));
                    appMap[QStringLiteral("exec")] = exec;
                } else if (line.startsWith(QLatin1String("Icon="))) {
                    appMap[QStringLiteral("icon")] = line.mid(5).trimmed();
                }
            }
            list.append(appMap);
        }
    }

    return list;
}

bool AppImageInstaller::uninstallAppImage(const QString& appName) {
    QString safeName = appName.toLower();
    safeName.replace(QRegularExpression(QStringLiteral("[^a-z0-9_-]")), QStringLiteral("-"));

    QString desktopPath = QDir::homePath() + QStringLiteral("/.local/share/applications/appimage-") + safeName + QStringLiteral(".desktop");
    QString binPath = QDir::homePath() + QStringLiteral("/.local/bin/") + safeName + QStringLiteral(".AppImage");

    bool success = false;
    if (QFile::exists(desktopPath)) {
        success |= QFile::remove(desktopPath);
    }
    if (QFile::exists(binPath)) {
        success |= QFile::remove(binPath);
    }

    QProcess::startDetached(QStringLiteral("update-desktop-database"), {QDir::homePath() + QStringLiteral("/.local/share/applications")});
    return success;
}

void AppImageInstaller::launchAppImage(const QString& appName) {
    QString safeName = appName.toLower();
    safeName.replace(QRegularExpression(QStringLiteral("[^a-z0-9_-]")), QStringLiteral("-"));
    QString binPath = QDir::homePath() + QStringLiteral("/.local/bin/") + safeName + QStringLiteral(".AppImage");
    if (QFile::exists(binPath)) {
        QProcess::startDetached(binPath);
    } else {
        QString altBinPath = QDir::homePath() + QStringLiteral("/.local/bin/") + appName;
        if (QFile::exists(altBinPath)) {
            QProcess::startDetached(altBinPath);
        }
    }
}
