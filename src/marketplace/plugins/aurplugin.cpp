#include "aurplugin.hpp"
#include "pluginprocess.hpp"
#include <QProcess>
#include <QFile>
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

namespace {
constexpr int kQueryTimeoutMs = 15000;
constexpr int kUpdatesTimeoutMs = 30000;
constexpr int kNetworkTimeoutMs = 10000;
}

AurPlugin::AurPlugin(QObject* parent) : IPackagePlugin(parent) {
    m_enabled = m_settings.value(QStringLiteral("AUR/enabled"), true).toBool();
    m_aurHelper = m_settings.value(QStringLiteral("AUR/helper"), QStringLiteral("auto")).toString();
}

bool AurPlugin::isAvailable() const {
    return !resolveHelper().isEmpty();
}

QString AurPlugin::resolveHelper() const {
    if (!m_aurHelper.isEmpty() && m_aurHelper != QStringLiteral("auto")) {
        if (!QStandardPaths::findExecutable(m_aurHelper).isEmpty()) return m_aurHelper;
    }
    for (const QString& candidate : {QStringLiteral("paru"), QStringLiteral("yay")}) {
        if (!QStandardPaths::findExecutable(candidate).isEmpty()) return candidate;
    }
    return QString();
}

void AurPlugin::setEnabled(bool enabled) {
    if (m_enabled != enabled) {
        m_enabled = enabled;
        m_settings.setValue(QStringLiteral("AUR/enabled"), enabled);
        emit enabledChanged();
    }
}

void AurPlugin::setAurHelper(const QString& helper) {
    if (m_aurHelper != helper) {
        m_aurHelper = helper;
        m_settings.setValue(QStringLiteral("AUR/helper"), helper);
        emit availabilityChanged();
    }
}

QVariantList AurPlugin::search(const QString& query, const QVariantMap& options) {
    Q_UNUSED(options);
    QVariantList results;
    if (!m_enabled) return results;

    QString q = query.trimmed();
    if (q.isEmpty()) return results;

    QNetworkAccessManager nam;
    QUrl url(QStringLiteral("https://aur.archlinux.org/rpc/v5/search/") + QUrl::toPercentEncoding(q));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("AstraMarket/1.0"));
    req.setTransferTimeout(kNetworkTimeoutMs);

    QEventLoop loop;
    QNetworkReply* reply = nam.get(req);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject() && doc.object().contains(QStringLiteral("results"))) {
            QJsonArray arr = doc.object()[QStringLiteral("results")].toArray();
            int count = 0;
            for (const QJsonValue& val : arr) {
                if (++count > 50) break;
                QJsonObject obj = val.toObject();
                QVariantMap item;
                item[QStringLiteral("id")] = obj[QStringLiteral("Name")].toString();
                item[QStringLiteral("name")] = obj[QStringLiteral("Name")].toString();
                item[QStringLiteral("version")] = obj[QStringLiteral("Version")].toString();
                item[QStringLiteral("summary")] = obj[QStringLiteral("Description")].toString();
                item[QStringLiteral("backend")] = QStringLiteral("AUR");
                item[QStringLiteral("repository")] = QStringLiteral("aur");
                item[QStringLiteral("scope")] = QStringLiteral("system");
                item[QStringLiteral("icon")] = obj[QStringLiteral("Name")].toString();
                results.append(item);
            }
        }
    }
    reply->deleteLater();
    return results;
}

QVariantList AurPlugin::parseForeignOutput(const QString& output) {
    QVariantList results;
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        const QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (parts.size() < 2) continue;

        QVariantMap item;
        item[QStringLiteral("id")] = parts.value(0);
        item[QStringLiteral("name")] = parts.value(0);
        item[QStringLiteral("version")] = parts.value(1);
        item[QStringLiteral("backend")] = QStringLiteral("AUR");
        item[QStringLiteral("repository")] = QStringLiteral("local");
        item[QStringLiteral("scope")] = QStringLiteral("system");
        item[QStringLiteral("icon")] = parts.value(0);
        item[QStringLiteral("isInstalled")] = true;
        results.append(item);
    }
    return results;
}

QVariantList AurPlugin::parseUpdatesOutput(const QString& output) {
    QVariantList results;
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        const QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (parts.size() < 4) continue;

        QVariantMap item;
        item[QStringLiteral("id")] = parts.value(0);
        item[QStringLiteral("name")] = parts.value(0);
        item[QStringLiteral("version")] = parts.value(1) + QStringLiteral(" -> ") + parts.value(3);
        item[QStringLiteral("backend")] = QStringLiteral("AUR");
        item[QStringLiteral("scope")] = QStringLiteral("system");
        item[QStringLiteral("icon")] = parts.value(0);
        results.append(item);
    }
    return results;
}

QVariantList AurPlugin::getInstalled() {
    QVariantList results;
    if (!m_enabled) return results;

    const astra::ProcessResult result = astra::runProcess(QStringLiteral("pacman"), {QStringLiteral("-Qm")}, kQueryTimeoutMs);
    results.append(parseForeignOutput(result.output));
    return results;
}

QVariantList AurPlugin::getUpdates() {
    QVariantList results;
    if (!isAvailable() || !m_enabled) return results;

    QString helper = resolveHelper();
    if (helper.isEmpty()) return results;

    const astra::ProcessResult result = astra::runProcess(helper, {QStringLiteral("-Qua")}, kUpdatesTimeoutMs);
    results.append(parseUpdatesOutput(result.output));
    return results;
}

QVariantMap AurPlugin::getDetails(const QString& packageId) {
    QVariantMap map;
    map[QStringLiteral("id")] = packageId;
    map[QStringLiteral("name")] = packageId;
    map[QStringLiteral("backend")] = QStringLiteral("AUR");

    QNetworkAccessManager nam;
    QUrl url(QStringLiteral("https://aur.archlinux.org/rpc/v5/info/") + QUrl::toPercentEncoding(packageId));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("AstraMarket/1.0"));
    req.setTransferTimeout(kNetworkTimeoutMs);

    QEventLoop loop;
    QNetworkReply* reply = nam.get(req);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject() && doc.object().contains(QStringLiteral("results"))) {
            QJsonArray arr = doc.object()[QStringLiteral("results")].toArray();
            if (!arr.isEmpty()) {
                QJsonObject obj = arr.first().toObject();
                map[QStringLiteral("summary")] = obj[QStringLiteral("Description")].toString();
                map[QStringLiteral("description")] = obj[QStringLiteral("Description")].toString();
                map[QStringLiteral("version")] = obj[QStringLiteral("Version")].toString();
                map[QStringLiteral("homepage")] = obj[QStringLiteral("URL")].toString();
                map[QStringLiteral("developer")] = obj[QStringLiteral("Maintainer")].toString();
                if (obj.contains(QStringLiteral("License")) && obj[QStringLiteral("License")].isArray()) {
                    QStringList lic;
                    for (const auto& l : obj[QStringLiteral("License")].toArray()) lic.append(l.toString());
                    map[QStringLiteral("license")] = lic.join(QStringLiteral(", "));
                }
            }
        }
    }
    reply->deleteLater();
    return map;
}

bool AurPlugin::install(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    Q_UNUSED(options);
    QString helper = resolveHelper();
    if (helper.isEmpty()) return false;

    const astra::ProcessResult result = astra::runProcessStreaming(helper, {QStringLiteral("-S"), QStringLiteral("--noconfirm"), packageId}, [&progressCb](const QString& line) {
        if (progressCb) progressCb(50, line);
    });
    return result.succeeded();
}

bool AurPlugin::uninstall(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    Q_UNUSED(options);
    const astra::ProcessResult result = astra::runProcessStreaming(QStringLiteral("pkexec"), {QStringLiteral("pacman"), QStringLiteral("-Rns"), QStringLiteral("--noconfirm"), packageId}, [&progressCb](const QString& line) {
        if (progressCb) progressCb(50, line);
    });
    return result.succeeded();
}

bool AurPlugin::update(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    Q_UNUSED(options);
    const QString helper = resolveHelper();
    if (helper.isEmpty()) return false;

    const astra::ProcessResult result = astra::runProcessStreaming(
        helper,
        {QStringLiteral("-Syu"), QStringLiteral("--noconfirm"), packageId},
        [&progressCb](const QString& line) {
            if (progressCb) progressCb(50, line);
        });
    return result.succeeded();
}

bool AurPlugin::launch(const QString& packageId) {
    return QProcess::startDetached(packageId);
}
