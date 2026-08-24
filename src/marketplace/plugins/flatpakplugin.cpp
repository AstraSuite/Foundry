#include "flatpakplugin.hpp"
#include "pluginprocess.hpp"
#include <QProcess>
#include <QFile>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QUrl>
#include <QUrlQuery>

FlatpakPlugin::FlatpakPlugin(QObject* parent) : IPackagePlugin(parent) {
    m_enabled = m_settings.value(QStringLiteral("Flatpak/enabled"), true).toBool();
}

bool FlatpakPlugin::isAvailable() const {
    return !QStandardPaths::findExecutable(QStringLiteral("flatpak")).isEmpty();
}

namespace {
constexpr int kQueryTimeoutMs = 15000;
constexpr int kSearchTimeoutMs = 5000;
constexpr int kDetailsTimeoutMs = 10000;

int percentFromLine(const QString& line) {
    static const QRegularExpression percentPattern(QStringLiteral(R"((\d{1,3})%)"));
    const QRegularExpressionMatch match = percentPattern.match(line);
    return match.hasMatch() ? qBound(0, match.captured(1).toInt(), 100) : -1;
}

QString withoutProgressBar(QString line) {
    static const QRegularExpression barPattern(QStringLiteral(R"([\x{2588}\x{2591}\x{2593}\x{2592}\-=|]{2,})"));
    line.remove(barPattern);
    return line.simplified();
}
}

QString FlatpakPlugin::scopeArgument(const QString& scope) {
    return scope == QStringLiteral("system") ? QStringLiteral("--system") : QStringLiteral("--user");
}

QString FlatpakPlugin::resolveScope(const QString& packageId, const QVariantMap& options) {
    const QString requested = options.value(QStringLiteral("scope")).toString();
    if (requested == QStringLiteral("user") || requested == QStringLiteral("system")) return requested;

    const QStringList scopes{QStringLiteral("user"), QStringLiteral("system")};
    for (const QString& scope : scopes) {
        const astra::ProcessResult result = astra::runProcess(
            QStringLiteral("flatpak"),
            {QStringLiteral("list"), QStringLiteral("--columns=application"), scopeArgument(scope)},
            kQueryTimeoutMs);

        const QStringList lines = result.output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (const QString& line : lines) {
            if (line.trimmed().compare(packageId, Qt::CaseInsensitive) == 0) return scope;
        }
    }
    return QStringLiteral("user");
}

QVariantList FlatpakPlugin::parseSearchOutput(const QString& output, QSet<QString>& seen) {
    QVariantList results;
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        const QStringList parts = line.split(QLatin1Char('\t'), Qt::KeepEmptyParts);
        if (parts.size() < 2) continue;

        const QString appId = parts.value(0).trimmed();
        if (appId.isEmpty() || seen.contains(appId)) continue;
        seen.insert(appId);

        QVariantMap item;
        item[QStringLiteral("id")] = appId;
        item[QStringLiteral("name")] = parts.value(1).trimmed();
        item[QStringLiteral("summary")] = parts.size() > 2 ? parts.value(2).trimmed() : QString();
        item[QStringLiteral("backend")] = QStringLiteral("Flatpak");
        item[QStringLiteral("scope")] = QStringLiteral("user");
        item[QStringLiteral("icon")] = appId;
        results.append(item);
    }
    return results;
}

QVariantList FlatpakPlugin::parseInstalledOutput(const QString& output, const QString& scope) {
    QVariantList results;
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        const QStringList parts = line.split(QLatin1Char('\t'), Qt::KeepEmptyParts);
        if (parts.size() < 2) continue;

        const QString appId = parts.value(0).trimmed();
        if (appId.isEmpty()) continue;

        QVariantMap item;
        item[QStringLiteral("id")] = appId;
        item[QStringLiteral("name")] = parts.value(1).trimmed().isEmpty() ? appId : parts.value(1).trimmed();
        const QString version = parts.value(2).trimmed();
        item[QStringLiteral("version")] = version.isEmpty() ? QStringLiteral("latest") : version;
        item[QStringLiteral("size")] = parts.value(3).trimmed();
        item[QStringLiteral("backend")] = QStringLiteral("Flatpak");
        item[QStringLiteral("scope")] = scope;
        item[QStringLiteral("icon")] = appId;
        item[QStringLiteral("isInstalled")] = true;
        results.append(item);
    }
    return results;
}

QVariantList FlatpakPlugin::parseUpdatesOutput(const QString& output) {
    QVariantList results;
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        const QStringList parts = line.split(QLatin1Char('\t'), Qt::KeepEmptyParts);
        if (parts.size() < 2) continue;

        const QString appId = parts.value(0).trimmed();
        if (appId.isEmpty()) continue;

        QVariantMap item;
        item[QStringLiteral("id")] = appId;
        item[QStringLiteral("name")] = parts.value(1).trimmed().isEmpty() ? appId : parts.value(1).trimmed();
        const QString version = parts.value(2).trimmed();
        item[QStringLiteral("version")] = version.isEmpty() ? QStringLiteral("update") : version;
        item[QStringLiteral("backend")] = QStringLiteral("Flatpak");
        item[QStringLiteral("scope")] = QStringLiteral("user");
        item[QStringLiteral("icon")] = appId;
        results.append(item);
    }
    return results;
}

void FlatpakPlugin::setEnabled(bool enabled) {
    if (m_enabled != enabled) {
        m_enabled = enabled;
        m_settings.setValue(QStringLiteral("Flatpak/enabled"), enabled);
        emit enabledChanged();
    }
}

QList<QVariantMap> FlatpakPlugin::getInstallSources(const QString& packageId) {
    Q_UNUSED(packageId);
    QList<QVariantMap> sources;
    QVariantMap userSource;
    userSource[QStringLiteral("id")] = QStringLiteral("user");
    userSource[QStringLiteral("label")] = QStringLiteral("Flathub (User)");
    userSource[QStringLiteral("description")] = QStringLiteral("Installed into user directory (~/.local/share/flatpak)");
    sources.append(userSource);

    QVariantMap systemSource;
    systemSource[QStringLiteral("id")] = QStringLiteral("system");
    systemSource[QStringLiteral("label")] = QStringLiteral("Flathub (System)");
    systemSource[QStringLiteral("description")] = QStringLiteral("Installed system-wide (/var/lib/flatpak)");
    sources.append(systemSource);

    return sources;
}

QVariantList FlatpakPlugin::parseCollectionHits(const QByteArray& payload, QSet<QString>& seen) {
    QVariantList results;

    const QJsonDocument document = QJsonDocument::fromJson(payload);
    if (!document.isObject() || !document.object().contains(QStringLiteral("hits"))) return results;

    const QJsonArray hits = document.object()[QStringLiteral("hits")].toArray();
    for (const QJsonValue& value : hits) {
        const QJsonObject object = value.toObject();
        const QString appId = object.contains(QStringLiteral("app_id")) ? object[QStringLiteral("app_id")].toString() : object[QStringLiteral("id")].toString();
        if (appId.isEmpty() || seen.contains(appId)) continue;
        seen.insert(appId);

        QVariantMap item;
        item[QStringLiteral("id")] = appId;
        item[QStringLiteral("name")] = object.contains(QStringLiteral("name")) ? object[QStringLiteral("name")].toString() : appId;
        item[QStringLiteral("summary")] = object[QStringLiteral("summary")].toString();
        item[QStringLiteral("backend")] = QStringLiteral("Flatpak");
        item[QStringLiteral("scope")] = QStringLiteral("user");
        item[QStringLiteral("icon")] = object[QStringLiteral("icon")].toString().isEmpty() ? appId : object[QStringLiteral("icon")].toString();
        item[QStringLiteral("developer")] = object[QStringLiteral("developer_name")].toString();
        item[QStringLiteral("verified")] = object.value(QStringLiteral("verification_verified")).toBool();
        results.append(item);
    }
    return results;
}

QVariantList FlatpakPlugin::getCollection(const QString& collection, int limit) {
    if (!isAvailable() || !m_enabled) return {};

    QNetworkAccessManager manager;
    QUrl url(QStringLiteral("https://flathub.org/api/v2/collection/") + collection);
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("page"), QStringLiteral("1"));
    query.addQueryItem(QStringLiteral("per_page"), QString::number(qBound(1, limit, 50)));
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("astra-foundry/1.0"));
    request.setTransferTimeout(kDetailsTimeoutMs);

    QEventLoop loop;
    QNetworkReply* reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QVariantList results;
    if (reply->error() == QNetworkReply::NoError) {
        QSet<QString> seen;
        results = parseCollectionHits(reply->readAll(), seen);
    }
    reply->deleteLater();
    return results;
}

QVariantList FlatpakPlugin::search(const QString& query, const QVariantMap& options) {
    Q_UNUSED(options);
    QVariantList results;
    if (!isAvailable() || !m_enabled) return results;

    QString q = query.trimmed();
    if (q.isEmpty()) return results;

    QSet<QString> seen;

    // Check if searching for a category collection
    static const QMap<QString, QStringList> categoryMap = {
        { QStringLiteral("Photo & Video"), { QStringLiteral("Graphics"), QStringLiteral("AudioVideo") } },
        { QStringLiteral("Music & Audio"), { QStringLiteral("AudioVideo") } },
        { QStringLiteral("Productivity"), { QStringLiteral("Office") } },
        { QStringLiteral("Communication & News"), { QStringLiteral("Network") } },
        { QStringLiteral("Education & Science"), { QStringLiteral("Science") } },
        { QStringLiteral("Games"), { QStringLiteral("Game") } },
        { QStringLiteral("Utilities"), { QStringLiteral("Utility") } },
        { QStringLiteral("Development"), { QStringLiteral("Development") } },
        { QStringLiteral("AudioVideo"), { QStringLiteral("AudioVideo") } },
        { QStringLiteral("Graphics"), { QStringLiteral("Graphics") } },
        { QStringLiteral("Network"), { QStringLiteral("Network") } },
        { QStringLiteral("Office"), { QStringLiteral("Office") } },
        { QStringLiteral("Game"), { QStringLiteral("Game") } },
        { QStringLiteral("Utility"), { QStringLiteral("Utility") } },
        { QStringLiteral("Science"), { QStringLiteral("Science") } }
    };

    bool isCategoryQuery = categoryMap.contains(q);
    QStringList targetCategories = isCategoryQuery ? categoryMap.value(q) : QStringList();

    if (isCategoryQuery) {
        for (const QString& cat : targetCategories) {
            QNetworkAccessManager nam;
            QUrl url(QStringLiteral("https://flathub.org/api/v2/collection/category/") + cat);
            QNetworkRequest req(url);
            req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("astra-foundry/1.0"));
            req.setTransferTimeout(kSearchTimeoutMs);

            QEventLoop loop;
            QNetworkReply* reply = nam.get(req);
            QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
            loop.exec();

            if (reply->error() == QNetworkReply::NoError) {
                results.append(parseCollectionHits(reply->readAll(), seen));
            }
            reply->deleteLater();
        }
    } else {
        // Query Flathub Search API (POST /api/v2/search)
        QNetworkAccessManager nam;
        QUrl url(QStringLiteral("https://flathub.org/api/v2/search"));
        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("astra-foundry/1.0"));
        req.setTransferTimeout(kSearchTimeoutMs);

        QJsonObject jsonBody;
        jsonBody[QStringLiteral("query")] = q;
        QByteArray payload = QJsonDocument(jsonBody).toJson(QJsonDocument::Compact);

        QEventLoop loop;
        QNetworkReply* reply = nam.post(req, payload);
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() == QNetworkReply::NoError) {
            results.append(parseCollectionHits(reply->readAll(), seen));
        }
        reply->deleteLater();
    }

    if (results.size() < 10) {
        const astra::ProcessResult local = astra::runProcess(
            QStringLiteral("flatpak"),
            {QStringLiteral("search"), q.toLower(), QStringLiteral("--columns=app,name,description")},
            kQueryTimeoutMs);
        results.append(parseSearchOutput(local.output, seen));
    }
    return results;
}

QVariantList FlatpakPlugin::getInstalled() {
    QVariantList results;
    if (!isAvailable() || !m_enabled) return results;

    const auto fetchScope = [](const QString& scope) -> QVariantList {
        const astra::ProcessResult result = astra::runProcess(
            QStringLiteral("flatpak"),
            {QStringLiteral("list"), QStringLiteral("--app"), QStringLiteral("--columns=app,name,version,size"),
             scope == QStringLiteral("user") ? QStringLiteral("--user") : QStringLiteral("--system")},
            kQueryTimeoutMs);
        return parseInstalledOutput(result.output, scope);
    };

    results.append(fetchScope(QStringLiteral("user")));
    results.append(fetchScope(QStringLiteral("system")));
    return results;
}

QVariantList FlatpakPlugin::getUpdates() {
    QVariantList results;
    if (!isAvailable() || !m_enabled) return results;

    const QStringList scopes{QStringLiteral("user"), QStringLiteral("system")};
    for (const QString& scope : scopes) {
        const astra::ProcessResult result = astra::runProcess(
            QStringLiteral("flatpak"),
            {QStringLiteral("remote-ls"), QStringLiteral("--updates"), QStringLiteral("--columns=app,name,version"), scopeArgument(scope)},
            kQueryTimeoutMs);

        const QVariantList updates = parseUpdatesOutput(result.output);
        for (const QVariant& value : updates) {
            QVariantMap item = value.toMap();
            item[QStringLiteral("scope")] = scope;
            results.append(item);
        }
    }
    return results;
}

QString FlatpakPlugin::formatBytes(qint64 bytes) {
    if (bytes <= 0) return QString();

    static const QStringList units{QStringLiteral("KB"), QStringLiteral("MB"), QStringLiteral("GB")};
    double value = bytes / 1024.0;
    int unit = 0;
    while (value >= 1024.0 && unit + 1 < units.size()) {
        value /= 1024.0;
        ++unit;
    }
    return QString::number(value, 'f', value < 10 ? 1 : 0) + QLatin1Char(' ') + units.at(unit);
}

namespace {

QByteArray httpGet(const QUrl& url, int timeoutMs) {
    QNetworkAccessManager manager;

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("astra-foundry/1.0"));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setTransferTimeout(timeoutMs);

    QEventLoop loop;
    QNetworkReply* reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QByteArray payload;
    if (reply->error() == QNetworkReply::NoError) payload = reply->readAll();
    reply->deleteLater();
    return payload;
}

int flaggedContentCategories(const QJsonObject& details) {
    for (const QString& locale : details.keys()) {
        const QJsonArray categories = details.value(locale).toObject().value(QStringLiteral("categories")).toArray();
        if (categories.isEmpty()) continue;

        int flagged = 0;
        for (const QJsonValue& value : categories) {
            if (value.toObject().value(QStringLiteral("level")).toString() != QStringLiteral("none")) ++flagged;
        }
        return flagged;
    }
    return -1;
}

void appendPermission(QVariantList& entries, const QString& label, const QString& icon, bool sensitive) {
    for (const QVariant& value : entries) {
        if (value.toMap().value(QStringLiteral("label")).toString() == label) return;
    }

    QVariantMap entry;
    entry[QStringLiteral("label")] = label;
    entry[QStringLiteral("icon")] = icon;
    entry[QStringLiteral("sensitive")] = sensitive;
    entries.append(entry);
}

void appendSharedPermission(QVariantList& entries, const QString& value) {
    if (value == QStringLiteral("network")) appendPermission(entries, QObject::tr("Network access"), QStringLiteral("public"), false);
}

void appendSocketPermission(QVariantList& entries, const QString& value) {
    if (value == QStringLiteral("x11")) appendPermission(entries, QObject::tr("Legacy X11 windowing system"), QStringLiteral("desktop_windows"), true);
    else if (value == QStringLiteral("wayland")) appendPermission(entries, QObject::tr("Wayland windowing system"), QStringLiteral("desktop_windows"), false);
    else if (value == QStringLiteral("fallback-x11")) appendPermission(entries, QObject::tr("X11 windowing system as fallback"), QStringLiteral("desktop_windows"), false);
    else if (value == QStringLiteral("pulseaudio")) appendPermission(entries, QObject::tr("Audio"), QStringLiteral("volume_up"), false);
    else if (value == QStringLiteral("session-bus")) appendPermission(entries, QObject::tr("Full session bus access"), QStringLiteral("hub"), true);
    else if (value == QStringLiteral("system-bus")) appendPermission(entries, QObject::tr("Full system bus access"), QStringLiteral("hub"), true);
    else if (value == QStringLiteral("ssh-auth")) appendPermission(entries, QObject::tr("SSH agent"), QStringLiteral("key"), true);
    else if (value == QStringLiteral("gpg-agent")) appendPermission(entries, QObject::tr("GPG agent"), QStringLiteral("key"), true);
    else if (value == QStringLiteral("cups")) appendPermission(entries, QObject::tr("Printing"), QStringLiteral("print"), false);
}

void appendDevicePermission(QVariantList& entries, const QString& value) {
    if (value == QStringLiteral("all")) appendPermission(entries, QObject::tr("All devices"), QStringLiteral("devices_other"), true);
    else if (value == QStringLiteral("dri")) appendPermission(entries, QObject::tr("GPU acceleration"), QStringLiteral("memory"), false);
    else if (value == QStringLiteral("input")) appendPermission(entries, QObject::tr("Input devices"), QStringLiteral("keyboard"), false);
    else if (value == QStringLiteral("kvm")) appendPermission(entries, QObject::tr("Virtual machines"), QStringLiteral("developer_board"), true);
    else if (value == QStringLiteral("shm")) appendPermission(entries, QObject::tr("Shared memory"), QStringLiteral("memory"), false);
}

void appendFilesystemPermission(QVariantList& entries, const QString& value) {
    QString path = value;
    bool readOnly = false;
    if (path.endsWith(QStringLiteral(":ro"))) {
        path.chop(3);
        readOnly = true;
    } else if (path.endsWith(QStringLiteral(":rw")) || path.endsWith(QStringLiteral(":create"))) {
        path = path.section(QLatin1Char(':'), 0, 0);
    }

    if (path == QStringLiteral("host")) {
        appendPermission(entries, readOnly ? QObject::tr("All system files, read only") : QObject::tr("All system files"), QStringLiteral("folder_open"), !readOnly);
    } else if (path == QStringLiteral("host-os")) {
        appendPermission(entries, QObject::tr("System libraries and binaries"), QStringLiteral("folder_open"), true);
    } else if (path == QStringLiteral("host-etc")) {
        appendPermission(entries, QObject::tr("System configuration in /etc"), QStringLiteral("folder_open"), true);
    } else if (path == QStringLiteral("home")) {
        appendPermission(entries, readOnly ? QObject::tr("Your home folder, read only") : QObject::tr("Your home folder"), QStringLiteral("home"), !readOnly);
    } else if (path.startsWith(QStringLiteral("xdg-"))) {
        appendPermission(entries, QObject::tr("Folder: %1").arg(path.mid(4)), QStringLiteral("folder"), false);
    } else if (!path.isEmpty()) {
        appendPermission(entries, QObject::tr("Path: %1").arg(path), QStringLiteral("folder"), !readOnly && !path.startsWith(QLatin1Char('~')));
    }
}

}

QVariantList FlatpakPlugin::permissionEntries(const QJsonObject& permissions) {
    QVariantList entries;

    for (const QJsonValue& value : permissions.value(QStringLiteral("shared")).toArray()) {
        appendSharedPermission(entries, value.toString());
    }
    for (const QJsonValue& value : permissions.value(QStringLiteral("sockets")).toArray()) {
        appendSocketPermission(entries, value.toString());
    }
    for (const QJsonValue& value : permissions.value(QStringLiteral("devices")).toArray()) {
        appendDevicePermission(entries, value.toString());
    }
    for (const QJsonValue& value : permissions.value(QStringLiteral("filesystems")).toArray()) {
        appendFilesystemPermission(entries, value.toString());
    }

    return entries;
}

QVariantList FlatpakPlugin::parseLocalPermissions(const QString& output) {
    QVariantList entries;

    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        const QString trimmed = line.trimmed();
        const qsizetype separator = trimmed.indexOf(QLatin1Char('='));
        if (separator <= 0) continue;

        const QString key = trimmed.left(separator);
        const QStringList values = trimmed.mid(separator + 1).split(QLatin1Char(';'), Qt::SkipEmptyParts);

        for (const QString& value : values) {
            if (key == QStringLiteral("shared")) appendSharedPermission(entries, value);
            else if (key == QStringLiteral("sockets")) appendSocketPermission(entries, value);
            else if (key == QStringLiteral("devices")) appendDevicePermission(entries, value);
            else if (key == QStringLiteral("filesystems")) appendFilesystemPermission(entries, value);
        }
    }

    return entries;
}

QVariantMap FlatpakPlugin::getDetails(const QString& packageId) {
    QVariantMap map;
    map[QStringLiteral("id")] = packageId;
    map[QStringLiteral("backend")] = QStringLiteral("Flatpak");
    map[QStringLiteral("name")] = packageId;
    map[QStringLiteral("summary")] = QStringLiteral("");
    map[QStringLiteral("description")] = QStringLiteral("");
    map[QStringLiteral("version")] = QStringLiteral("");
    map[QStringLiteral("developer")] = QStringLiteral("");
    map[QStringLiteral("license")] = QStringLiteral("");
    map[QStringLiteral("homepage")] = QStringLiteral("");
    map[QStringLiteral("iconUrl")] = QStringLiteral("");
    map[QStringLiteral("screenshots")] = QVariantList();

    QNetworkAccessManager nam;
    QUrl url(QStringLiteral("https://flathub.org/api/v2/appstream/") + packageId);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("astra-foundry/1.0"));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    req.setTransferTimeout(kDetailsTimeoutMs);

    QEventLoop loop;
    QNetworkReply* reply = nam.get(req);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains(QStringLiteral("name"))) map[QStringLiteral("name")] = obj[QStringLiteral("name")].toString();
            if (obj.contains(QStringLiteral("summary"))) map[QStringLiteral("summary")] = obj[QStringLiteral("summary")].toString();
            if (obj.contains(QStringLiteral("description"))) map[QStringLiteral("description")] = obj[QStringLiteral("description")].toString();
            if (obj.contains(QStringLiteral("developer_name"))) map[QStringLiteral("developer")] = obj[QStringLiteral("developer_name")].toString();
            if (obj.contains(QStringLiteral("project_license"))) map[QStringLiteral("license")] = obj[QStringLiteral("project_license")].toString();
            if (obj.contains(QStringLiteral("icon"))) map[QStringLiteral("iconUrl")] = obj[QStringLiteral("icon")].toString();
            if (obj.contains(QStringLiteral("icon_url"))) map[QStringLiteral("iconUrl")] = obj[QStringLiteral("icon_url")].toString();
            if (obj.contains(QStringLiteral("icon_128"))) map[QStringLiteral("iconUrl")] = obj[QStringLiteral("icon_128")].toString();

            if (obj.contains(QStringLiteral("urls")) && obj[QStringLiteral("urls")].isObject()) {
                QJsonObject urls = obj[QStringLiteral("urls")].toObject();
                if (urls.contains(QStringLiteral("homepage"))) map[QStringLiteral("homepage")] = urls[QStringLiteral("homepage")].toString();
            }

            if (obj.contains(QStringLiteral("screenshots")) && obj[QStringLiteral("screenshots")].isArray()) {
                QVariantList shots;
                for (const QJsonValue& val : obj[QStringLiteral("screenshots")].toArray()) {
                    if (val.isObject()) {
                        QJsonObject sObj = val.toObject();
                        if (sObj.contains(QStringLiteral("img_mobile"))) {
                            shots.append(sObj[QStringLiteral("img_mobile")].toString());
                        } else if (sObj.contains(QStringLiteral("img_desktop"))) {
                            shots.append(sObj[QStringLiteral("img_desktop")].toString());
                        }
                    }
                }
                map[QStringLiteral("screenshots")] = shots;
            }

            const int flagged = flaggedContentCategories(obj.value(QStringLiteral("content_rating_details")).toObject());
            if (flagged >= 0) map[QStringLiteral("contentRatingFlags")] = flagged;
        }
    }
    reply->deleteLater();

    const QJsonDocument summary = QJsonDocument::fromJson(
        httpGet(QUrl(QStringLiteral("https://flathub.org/api/v2/summary/") + packageId), kDetailsTimeoutMs));
    if (summary.isObject()) {
        const QJsonObject root = summary.object();
        const QString downloadSize = formatBytes(root.value(QStringLiteral("download_size")).toInteger());
        const QString installedSize = formatBytes(root.value(QStringLiteral("installed_size")).toInteger());
        if (!downloadSize.isEmpty()) map[QStringLiteral("downloadSize")] = downloadSize;
        if (!installedSize.isEmpty()) map[QStringLiteral("installedSize")] = installedSize;

        const QJsonObject metadata = root.value(QStringLiteral("metadata")).toObject();
        const QVariantList permissions = permissionEntries(metadata.value(QStringLiteral("permissions")).toObject());
        if (!permissions.isEmpty()) map[QStringLiteral("permissions")] = permissions;

        const QString runtime = metadata.value(QStringLiteral("runtimeName")).toString();
        if (!runtime.isEmpty()) map[QStringLiteral("runtime")] = runtime;
    }

    if (map.value(QStringLiteral("permissions")).toList().isEmpty()) {
        const astra::ProcessResult local = astra::runProcess(
            QStringLiteral("flatpak"), {QStringLiteral("info"), QStringLiteral("--show-permissions"), packageId}, kQueryTimeoutMs);
        if (local.succeeded()) {
            const QVariantList permissions = parseLocalPermissions(local.output);
            if (!permissions.isEmpty()) map[QStringLiteral("permissions")] = permissions;
        }
    }

    const QJsonDocument stats = QJsonDocument::fromJson(
        httpGet(QUrl(QStringLiteral("https://flathub.org/api/v2/stats/") + packageId), kSearchTimeoutMs));
    if (stats.isObject()) {
        const QJsonObject root = stats.object();
        const qint64 total = root.value(QStringLiteral("installs_total")).toInteger();
        const qint64 lastMonth = root.value(QStringLiteral("installs_last_month")).toInteger();
        if (total > 0) map[QStringLiteral("installsTotal")] = total;
        if (lastMonth > 0) map[QStringLiteral("installsLastMonth")] = lastMonth;
    }

    const QJsonDocument verification = QJsonDocument::fromJson(
        httpGet(QUrl(QStringLiteral("https://flathub.org/api/v2/verification/") + packageId + QStringLiteral("/status")), kSearchTimeoutMs));
    if (verification.isObject() && verification.object().contains(QStringLiteral("verified"))) {
        map[QStringLiteral("verified")] = verification.object().value(QStringLiteral("verified")).toBool();
    }

    return map;
}

bool FlatpakPlugin::install(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    if (!isAvailable()) return false;
    QStringList args;
    args << QStringLiteral("install")
         << scopeArgument(options.value(QStringLiteral("scope")).toString())
         << QStringLiteral("-y")
         << packageId;

    const astra::ProcessResult result = astra::runProcessStreaming(QStringLiteral("flatpak"), args, [&progressCb](const QString& line) {
        if (!progressCb) return;
        const QString message = withoutProgressBar(line);
        if (message.isEmpty()) return;
        const int percent = percentFromLine(line);
        progressCb(percent < 0 ? 50 : percent, message);
    }, 0, {}, m_cancellation);
    return result.succeeded();
}

bool FlatpakPlugin::uninstall(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    if (!isAvailable()) return false;
    QStringList args;
    args << QStringLiteral("uninstall")
         << scopeArgument(resolveScope(packageId, options))
         << QStringLiteral("-y")
         << packageId;

    const astra::ProcessResult result = astra::runProcessStreaming(QStringLiteral("flatpak"), args, [&progressCb](const QString& line) {
        if (!progressCb) return;
        const QString message = withoutProgressBar(line);
        if (message.isEmpty()) return;
        const int percent = percentFromLine(line);
        progressCb(percent < 0 ? 50 : percent, message);
    }, 0, {}, m_cancellation);
    return result.succeeded();
}

bool FlatpakPlugin::update(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    if (!isAvailable()) return false;

    QStringList args;
    args << QStringLiteral("update")
         << scopeArgument(resolveScope(packageId, options))
         << QStringLiteral("-y")
         << packageId;

    const astra::ProcessResult result = astra::runProcessStreaming(QStringLiteral("flatpak"), args, [&progressCb](const QString& line) {
        if (!progressCb) return;
        const QString message = withoutProgressBar(line);
        if (message.isEmpty()) return;
        const int percent = percentFromLine(line);
        progressCb(percent < 0 ? 50 : percent, message);
    }, 0, {}, m_cancellation);
    return result.succeeded();
}

bool FlatpakPlugin::launch(const QString& packageId) {
    if (!isAvailable()) return false;
    return QProcess::startDetached(QStringLiteral("flatpak"), {QStringLiteral("run"), packageId});
}
