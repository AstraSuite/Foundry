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
            req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("AstraMarket/1.0"));
            req.setTransferTimeout(kSearchTimeoutMs);

            QEventLoop loop;
            QNetworkReply* reply = nam.get(req);
            QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
            loop.exec();

            if (reply->error() == QNetworkReply::NoError) {
                QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
                if (doc.isObject() && doc.object().contains(QStringLiteral("hits"))) {
                    QJsonArray hits = doc.object()[QStringLiteral("hits")].toArray();
                    for (const QJsonValue& val : hits) {
                        QJsonObject obj = val.toObject();
                        QString appId = obj.contains(QStringLiteral("app_id")) ? obj[QStringLiteral("app_id")].toString() : obj[QStringLiteral("id")].toString();
                        if (appId.isEmpty() || seen.contains(appId)) continue;
                        seen.insert(appId);

                        QVariantMap item;
                        item[QStringLiteral("id")] = appId;
                        item[QStringLiteral("name")] = obj.contains(QStringLiteral("name")) ? obj[QStringLiteral("name")].toString() : appId;
                        item[QStringLiteral("summary")] = obj.contains(QStringLiteral("summary")) ? obj[QStringLiteral("summary")].toString() : QString();
                        item[QStringLiteral("backend")] = QStringLiteral("Flatpak");
                        item[QStringLiteral("scope")] = QStringLiteral("user");
                        item[QStringLiteral("icon")] = obj.contains(QStringLiteral("icon")) && !obj[QStringLiteral("icon")].toString().isEmpty() ? obj[QStringLiteral("icon")].toString() : appId;
                        item[QStringLiteral("verified")] = obj.value(QStringLiteral("verification_verified")).toBool();
                        results.append(item);
                    }
                }
            }
            reply->deleteLater();
        }
    } else {
        // Query Flathub Search API (POST /api/v2/search)
        QNetworkAccessManager nam;
        QUrl url(QStringLiteral("https://flathub.org/api/v2/search"));
        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("AstraMarket/1.0"));
        req.setTransferTimeout(kSearchTimeoutMs);

        QJsonObject jsonBody;
        jsonBody[QStringLiteral("query")] = q;
        QByteArray payload = QJsonDocument(jsonBody).toJson(QJsonDocument::Compact);

        QEventLoop loop;
        QNetworkReply* reply = nam.post(req, payload);
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (doc.isObject() && doc.object().contains(QStringLiteral("hits"))) {
                QJsonArray hits = doc.object()[QStringLiteral("hits")].toArray();
                for (const QJsonValue& val : hits) {
                    QJsonObject obj = val.toObject();
                    QString appId = obj.contains(QStringLiteral("app_id")) ? obj[QStringLiteral("app_id")].toString() : obj[QStringLiteral("id")].toString();
                    if (appId.isEmpty() || seen.contains(appId)) continue;
                    seen.insert(appId);

                    QVariantMap item;
                    item[QStringLiteral("id")] = appId;
                    item[QStringLiteral("name")] = obj.contains(QStringLiteral("name")) ? obj[QStringLiteral("name")].toString() : appId;
                    item[QStringLiteral("summary")] = obj.contains(QStringLiteral("summary")) ? obj[QStringLiteral("summary")].toString() : QString();
                    item[QStringLiteral("backend")] = QStringLiteral("Flatpak");
                    item[QStringLiteral("scope")] = QStringLiteral("user");
                    item[QStringLiteral("icon")] = obj.contains(QStringLiteral("icon")) && !obj[QStringLiteral("icon")].toString().isEmpty() ? obj[QStringLiteral("icon")].toString() : appId;
                    item[QStringLiteral("verified")] = obj.value(QStringLiteral("verification_verified")).toBool();
                    results.append(item);
                }
            }
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

    const astra::ProcessResult result = astra::runProcess(
        QStringLiteral("flatpak"),
        {QStringLiteral("remote-ls"), QStringLiteral("--updates"), QStringLiteral("--columns=app,name,version")},
        kQueryTimeoutMs);
    results.append(parseUpdatesOutput(result.output));
    return results;
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
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("AstraMarket/1.0"));
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
        }
    }
    reply->deleteLater();
    return map;
}

bool FlatpakPlugin::install(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    if (!isAvailable()) return false;
    QString scope = options.value(QStringLiteral("scope"), QStringLiteral("user")).toString();
    QStringList args;
    args << QStringLiteral("install")
         << (scope == QStringLiteral("system") ? QStringLiteral("--system") : QStringLiteral("--user"))
         << QStringLiteral("-y")
         << packageId;

    const astra::ProcessResult result = astra::runProcessStreaming(QStringLiteral("flatpak"), args, [&progressCb](const QString& line) {
        if (!progressCb) return;
        const QString message = withoutProgressBar(line);
        if (message.isEmpty()) return;
        const int percent = percentFromLine(line);
        progressCb(percent < 0 ? 50 : percent, message);
    });
    return result.succeeded();
}

bool FlatpakPlugin::uninstall(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    Q_UNUSED(options);
    if (!isAvailable()) return false;
    QStringList args;
    args << QStringLiteral("uninstall") << QStringLiteral("-y") << packageId;

    const astra::ProcessResult result = astra::runProcessStreaming(QStringLiteral("flatpak"), args, [&progressCb](const QString& line) {
        if (!progressCb) return;
        const QString message = withoutProgressBar(line);
        if (message.isEmpty()) return;
        const int percent = percentFromLine(line);
        progressCb(percent < 0 ? 50 : percent, message);
    });
    return result.succeeded();
}

bool FlatpakPlugin::launch(const QString& packageId) {
    if (!isAvailable()) return false;
    return QProcess::startDetached(QStringLiteral("flatpak"), {QStringLiteral("run"), packageId});
}
