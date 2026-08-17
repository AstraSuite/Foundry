#include "flatpakplugin.hpp"
#include <QProcess>
#include <QFile>
#include <QSet>
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
    return QFile::exists(QStringLiteral("/usr/bin/flatpak")) || QFile::exists(QStringLiteral("/bin/flatpak"));
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
            req.setTransferTimeout(2500);

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
        req.setTransferTimeout(2500);

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

    // Local CLI flatpak search as fallback / supplemental
    if (results.size() < 10) {
        QProcess proc;
        proc.start(QStringLiteral("flatpak"), {QStringLiteral("search"), q.toLower(), QStringLiteral("--columns=app,name,description")});
        if (proc.waitForFinished(3000)) {
            QString output = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
            QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
            for (const QString& line : lines) {
                QStringList parts = line.split(QLatin1Char('\t'), Qt::KeepEmptyParts);
                if (parts.size() >= 2) {
                    QString appId = parts.value(0).trimmed();
                    QString appName = parts.value(1).trimmed();
                    if (appId.isEmpty() || seen.contains(appId)) continue;
                    seen.insert(appId);

                    QVariantMap item;
                    item[QStringLiteral("id")] = appId;
                    item[QStringLiteral("name")] = appName;
                    item[QStringLiteral("summary")] = parts.size() > 2 ? parts.value(2).trimmed() : QStringLiteral("");
                    item[QStringLiteral("backend")] = QStringLiteral("Flatpak");
                    item[QStringLiteral("scope")] = QStringLiteral("user");
                    item[QStringLiteral("icon")] = appId;
                    results.append(item);
                }
            }
        } else {
            proc.kill();
            proc.waitForFinished(300);
        }
    }
    return results;
}

QVariantList FlatpakPlugin::getInstalled() {
    QVariantList results;
    if (!isAvailable() || !m_enabled) return results;

    auto fetchScope = [](const QString& scope) -> QVariantList {
        QVariantList list;
        QProcess proc;
        proc.start(QStringLiteral("flatpak"), {QStringLiteral("list"), QStringLiteral("--app"), QStringLiteral("--columns=app,name,version,size"), scope == QStringLiteral("user") ? QStringLiteral("--user") : QStringLiteral("--system")});
        if (proc.waitForFinished(5000)) {
            QString output = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
            QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
            for (const QString& line : lines) {
                QStringList parts = line.split(QLatin1Char('\t'), Qt::KeepEmptyParts);
                if (parts.size() >= 2) {
                    QVariantMap item;
                    item[QStringLiteral("id")] = parts.value(0).trimmed();
                    item[QStringLiteral("name")] = parts.value(1).trimmed().isEmpty() ? parts.value(0).trimmed() : parts.value(1).trimmed();
                    item[QStringLiteral("version")] = parts.size() > 2 ? parts.value(2).trimmed() : QStringLiteral("latest");
                    item[QStringLiteral("size")] = parts.size() > 3 ? parts.value(3).trimmed() : QStringLiteral("");
                    item[QStringLiteral("backend")] = QStringLiteral("Flatpak");
                    item[QStringLiteral("scope")] = scope;
                    item[QStringLiteral("icon")] = parts.value(0).trimmed();
                    item[QStringLiteral("isInstalled")] = true;
                    list.append(item);
                }
            }
        } else {
            proc.kill();
            proc.waitForFinished(500);
        }
        return list;
    };

    results.append(fetchScope(QStringLiteral("user")));
    results.append(fetchScope(QStringLiteral("system")));
    return results;
}

QVariantList FlatpakPlugin::getUpdates() {
    QVariantList results;
    if (!isAvailable() || !m_enabled) return results;

    QProcess proc;
    proc.start(QStringLiteral("flatpak"), {QStringLiteral("remote-ls"), QStringLiteral("--updates"), QStringLiteral("--columns=app,name,version")});
    if (proc.waitForFinished(8000)) {
        QString output = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
        QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (const QString& line : lines) {
            QStringList parts = line.split(QLatin1Char('\t'), Qt::KeepEmptyParts);
            if (parts.size() >= 2) {
                QVariantMap item;
                item[QStringLiteral("id")] = parts.value(0).trimmed();
                item[QStringLiteral("name")] = parts.value(1).trimmed();
                item[QStringLiteral("version")] = parts.size() > 2 ? parts.value(2).trimmed() : QStringLiteral("update");
                item[QStringLiteral("backend")] = QStringLiteral("Flatpak");
                item[QStringLiteral("scope")] = QStringLiteral("user");
                item[QStringLiteral("icon")] = parts.value(0).trimmed();
                results.append(item);
            }
        }
    } else {
        proc.kill();
        proc.waitForFinished(500);
    }
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

    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start(QStringLiteral("flatpak"), args);
    if (!proc.waitForStarted(5000)) return false;

    while (proc.state() == QProcess::Running) {
        if (proc.waitForReadyRead(200)) {
            QByteArray data = proc.readAll();
            QString output = QString::fromUtf8(data);
            QStringList chunks = output.split(QRegularExpression(QStringLiteral("[\r\n]+")), Qt::SkipEmptyParts);
            for (QString chunk : chunks) {
                chunk = chunk.trimmed();
                if (chunk.isEmpty()) continue;

                int pct = 50;
                QRegularExpression pctRe(QStringLiteral(R"((\d{1,3})%)"));
                auto match = pctRe.match(chunk);
                if (match.hasMatch()) {
                    pct = match.captured(1).toInt();
                }

                chunk.remove(QRegularExpression(QStringLiteral(R"([█░▓▒\-=|]{2,})")));
                chunk = chunk.simplified();

                if (progressCb && !chunk.isEmpty()) {
                    progressCb(pct, chunk);
                }
            }
        }
    }
    return proc.exitCode() == 0;
}

bool FlatpakPlugin::uninstall(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    Q_UNUSED(options);
    if (!isAvailable()) return false;
    QStringList args;
    args << QStringLiteral("uninstall") << QStringLiteral("-y") << packageId;

    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start(QStringLiteral("flatpak"), args);
    if (!proc.waitForStarted(5000)) return false;

    while (proc.state() == QProcess::Running) {
        if (proc.waitForReadyRead(200)) {
            QByteArray data = proc.readAll();
            QString output = QString::fromUtf8(data);
            QStringList chunks = output.split(QRegularExpression(QStringLiteral("[\r\n]+")), Qt::SkipEmptyParts);
            for (QString chunk : chunks) {
                chunk = chunk.trimmed();
                if (chunk.isEmpty()) continue;

                int pct = 50;
                QRegularExpression pctRe(QStringLiteral(R"((\d{1,3})%)"));
                auto match = pctRe.match(chunk);
                if (match.hasMatch()) {
                    pct = match.captured(1).toInt();
                }

                chunk.remove(QRegularExpression(QStringLiteral(R"([█░▓▒\-=|]{2,})")));
                chunk = chunk.simplified();

                if (progressCb && !chunk.isEmpty()) {
                    progressCb(pct, chunk);
                }
            }
        }
    }
    return proc.exitCode() == 0;
}

bool FlatpakPlugin::launch(const QString& packageId) {
    if (!isAvailable()) return false;
    return QProcess::startDetached(QStringLiteral("flatpak"), {QStringLiteral("run"), packageId});
}
