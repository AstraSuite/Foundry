#include "appimageplugin.hpp"
#include "appimageinstaller.hpp"
#include "pluginprocess.hpp"

#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QDir>
#include <QSet>
#include <QUrl>

namespace {
constexpr int kCheckTimeoutMs = 30000;
constexpr int kCatalogTimeoutMs = 20000;
constexpr int kMaxCatalogResults = 40;

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

QVariantList AppImagePlugin::parseCatalog(const QByteArray& payload) {
    QVariantList entries;

    const QJsonDocument document = QJsonDocument::fromJson(payload);
    if (!document.isObject()) return entries;

    const QJsonArray items = document.object().value(QStringLiteral("items")).toArray();
    for (const QJsonValue& value : items) {
        const QJsonObject item = value.toObject();
        const QString name = item.value(QStringLiteral("name")).toString();
        if (name.isEmpty()) continue;

        QString repository;
        QString releasePage;
        for (const QJsonValue& linkValue : item.value(QStringLiteral("links")).toArray()) {
            const QJsonObject link = linkValue.toObject();
            const QString type = link.value(QStringLiteral("type")).toString();
            if (type == QStringLiteral("GitHub")) repository = link.value(QStringLiteral("url")).toString();
            else if (type == QStringLiteral("Download")) releasePage = link.value(QStringLiteral("url")).toString();
        }
        if (repository.isEmpty()) continue;

        QVariantMap entry;
        entry[QStringLiteral("id")] = name;
        entry[QStringLiteral("name")] = name;
        entry[QStringLiteral("summary")] = item.value(QStringLiteral("description")).toString();
        entry[QStringLiteral("backend")] = QStringLiteral("AppImage");
        entry[QStringLiteral("scope")] = QStringLiteral("user");
        entry[QStringLiteral("repository")] = QStringLiteral("AppImageHub");
        entry[QStringLiteral("catalog")] = true;
        entry[QStringLiteral("githubRepository")] = repository;
        entry[QStringLiteral("homepage")] = QStringLiteral("https://github.com/") + repository;
        entry[QStringLiteral("releasePage")] = releasePage;

        const QJsonArray icons = item.value(QStringLiteral("icons")).toArray();
        entry[QStringLiteral("icon")] = icons.isEmpty()
            ? name
            : QStringLiteral("https://appimage.github.io/database/") + icons.first().toString();

        const QJsonArray authors = item.value(QStringLiteral("authors")).toArray();
        if (!authors.isEmpty()) entry[QStringLiteral("developer")] = authors.first().toObject().value(QStringLiteral("name")).toString();

        const QString license = item.value(QStringLiteral("license")).toString();
        if (!license.isEmpty()) entry[QStringLiteral("license")] = license.section(QLatin1Char('='), 0, 0);

        entries.append(entry);
    }
    return entries;
}

QVariantList AppImagePlugin::getCatalog() {
    QMutexLocker locker(&m_catalogMutex);
    if (m_catalogLoaded) return m_catalog;
    locker.unlock();

    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(QStringLiteral("https://appimage.github.io/feed.json")));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("astra-foundry/1.0"));
    request.setTransferTimeout(kCatalogTimeoutMs);

    QEventLoop loop;
    QNetworkReply* reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QVariantList entries;
    if (reply->error() == QNetworkReply::NoError) entries = parseCatalog(reply->readAll());
    reply->deleteLater();

    locker.relock();
    if (!entries.isEmpty()) {
        m_catalog = entries;
        m_catalogLoaded = true;
    }
    return m_catalog;
}

QVariantMap AppImagePlugin::catalogEntry(const QString& packageId) {
    const QVariantList catalog = getCatalog();
    for (const QVariant& value : catalog) {
        const QVariantMap entry = value.toMap();
        if (entry.value(QStringLiteral("id")).toString().compare(packageId, Qt::CaseInsensitive) == 0) return entry;
    }
    return {};
}

QString AppImagePlugin::pickReleaseAsset(const QByteArray& payload, QString* fileName) {
    const QJsonDocument document = QJsonDocument::fromJson(payload);
    if (!document.isObject()) return QString();

    QString fallbackUrl;
    QString fallbackName;

    for (const QJsonValue& value : document.object().value(QStringLiteral("assets")).toArray()) {
        const QJsonObject asset = value.toObject();
        const QString name = asset.value(QStringLiteral("name")).toString();
        if (!name.endsWith(QLatin1String(".appimage"), Qt::CaseInsensitive)) continue;

        const QString url = asset.value(QStringLiteral("browser_download_url")).toString();
        const QString lowered = name.toLower();
        if (lowered.contains(QStringLiteral("aarch64")) || lowered.contains(QStringLiteral("arm64")) || lowered.contains(QStringLiteral("armhf"))) continue;

        if (lowered.contains(QStringLiteral("x86_64")) || lowered.contains(QStringLiteral("amd64"))) {
            if (fileName) *fileName = name;
            return url;
        }

        if (fallbackUrl.isEmpty()) {
            fallbackUrl = url;
            fallbackName = name;
        }
    }

    if (fileName) *fileName = fallbackName;
    return fallbackUrl;
}

QString AppImagePlugin::resolveDownloadUrl(const QString& repository, QString* fileName) {
    if (repository.isEmpty()) return QString();

    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(QStringLiteral("https://api.github.com/repos/") + repository + QStringLiteral("/releases/latest")));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("astra-foundry/1.0"));
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setTransferTimeout(kCatalogTimeoutMs);

    QEventLoop loop;
    QNetworkReply* reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QString url;
    if (reply->error() == QNetworkReply::NoError) url = pickReleaseAsset(reply->readAll(), fileName);
    reply->deleteLater();
    return url;
}

QString AppImagePlugin::downloadRelease(const QString& url, const QString& fileName, ProgressCallback progressCb) {
    QTemporaryDir temporaryDir;
    temporaryDir.setAutoRemove(false);
    if (!temporaryDir.isValid()) return QString();

    const QString target = temporaryDir.path() + QStringLiteral("/") + (fileName.isEmpty() ? QStringLiteral("download.AppImage") : fileName);
    QFile file(target);
    if (!file.open(QIODevice::WriteOnly)) return QString();

    QNetworkAccessManager manager;
    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("astra-foundry/1.0"));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    QEventLoop loop;
    QNetworkReply* reply = manager.get(request);

    QObject::connect(reply, &QNetworkReply::readyRead, reply, [&file, reply, this] {
        file.write(reply->readAll());
        if (m_cancellation && m_cancellation->isCancelled()) reply->abort();
    });
    QObject::connect(reply, &QNetworkReply::downloadProgress, reply, [&progressCb, fileName](qint64 received, qint64 total) {
        if (!progressCb || total <= 0) return;
        const int percent = static_cast<int>((received * 100) / total);
        progressCb(percent, QObject::tr("Downloading %1 (%2%)").arg(fileName, QString::number(percent)));
    });
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const bool failed = reply->error() != QNetworkReply::NoError;
    file.write(reply->readAll());
    file.close();
    reply->deleteLater();

    if (failed) {
        QFile::remove(target);
        return QString();
    }
    return target;
}

QVariantList AppImagePlugin::search(const QString& query, const QVariantMap& options) {
    Q_UNUSED(options);
    QVariantList results;
    if (!m_enabled) return results;

    const QString needle = query.trimmed().toLower();
    if (needle.isEmpty()) return results;

    QSet<QString> seen;
    const QVariantList installed = getInstalled();
    for (const QVariant& value : installed) {
        const QVariantMap item = value.toMap();
        const QString id = item.value(QStringLiteral("id")).toString();
        if (item.value(QStringLiteral("name")).toString().toLower().contains(needle) || id.toLower().contains(needle)) {
            seen.insert(id.toLower());
            results.append(item);
        }
    }

    const QVariantList catalog = getCatalog();
    for (const QVariant& value : catalog) {
        if (results.size() >= kMaxCatalogResults) break;

        const QVariantMap item = value.toMap();
        const QString id = item.value(QStringLiteral("id")).toString();
        if (seen.contains(id.toLower())) continue;
        if (!id.toLower().contains(needle) && !item.value(QStringLiteral("summary")).toString().toLower().contains(needle)) continue;

        results.append(item);
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

    if (map.isEmpty()) {
        const QVariantMap entry = catalogEntry(packageId);
        if (!entry.isEmpty()) {
            map = entry;
            map[QStringLiteral("description")] = entry.value(QStringLiteral("summary"));
            map[QStringLiteral("version")] = tr("latest release");
            return map;
        }
    }

    map[QStringLiteral("id")] = map.value(QStringLiteral("id"), packageId);
    map[QStringLiteral("name")] = map.value(QStringLiteral("name"), packageId);
    map[QStringLiteral("backend")] = QStringLiteral("AppImage");
    map[QStringLiteral("version")] = QStringLiteral("portable");
    map[QStringLiteral("repository")] = QStringLiteral("local");
    if (map.contains(QStringLiteral("size"))) map[QStringLiteral("installedSize")] = map.value(QStringLiteral("size"));

    const QString summary = map.value(QStringLiteral("summary")).toString();
    if (summary.isEmpty()) {
        map[QStringLiteral("summary")] = tr("Standalone AppImage application");
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
    if (info.exists() && info.isFile()) {
        if (progressCb) progressCb(10, tr("Installing %1").arg(info.fileName()));

        AppImageInstaller installer;
        const bool installed = installer.installAppImage(info.absoluteFilePath());
        if (progressCb) progressCb(installed ? 100 : 0, installer.statusMessage());
        return installed;
    }

    const QVariantMap entry = catalogEntry(packageId);
    if (entry.isEmpty()) {
        if (progressCb) progressCb(0, tr("No AppImage file and no catalogue entry for %1").arg(packageId));
        return false;
    }

    if (progressCb) progressCb(5, tr("Looking up the latest release of %1").arg(packageId));

    QString fileName;
    const QString downloadUrl = resolveDownloadUrl(entry.value(QStringLiteral("githubRepository")).toString(), &fileName);
    if (downloadUrl.isEmpty()) {
        if (progressCb) progressCb(0, tr("The latest release of %1 has no AppImage for this architecture").arg(packageId));
        return false;
    }

    const QString downloaded = downloadRelease(downloadUrl, fileName, progressCb);
    if (downloaded.isEmpty()) {
        if (progressCb) progressCb(0, tr("Downloading %1 failed").arg(fileName));
        return false;
    }

    if (progressCb) progressCb(100, tr("Installing %1").arg(fileName));

    AppImageInstaller installer;
    const bool installed = installer.installAppImage(downloaded);
    QDir(QFileInfo(downloaded).absolutePath()).removeRecursively();

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
