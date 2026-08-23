#include "aurplugin.hpp"
#include "pluginprocess.hpp"
#include <algorithm>
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
constexpr int kSudoProbeTimeoutMs = 5000;

bool sudoIsAuthorised() {
    return astra::runProcess(QStringLiteral("sudo"), {QStringLiteral("-n"), QStringLiteral("true")}, kSudoProbeTimeoutMs).succeeded();
}

QString findAskpass() {
    const QString configured = qEnvironmentVariable("SUDO_ASKPASS");
    if (!configured.isEmpty() && QFile::exists(configured)) return configured;

    const QStringList candidates{
        QStringLiteral("/usr/bin/ksshaskpass"),
        QStringLiteral("/usr/bin/lxqt-openssh-askpass"),
        QStringLiteral("/usr/bin/ssh-askpass"),
        QStringLiteral("/usr/bin/x11-ssh-askpass"),
        QStringLiteral("/usr/lib/ssh/ssh-askpass"),
        QStringLiteral("/usr/lib/ssh/x11-ssh-askpass"),
        QStringLiteral("/usr/lib/seahorse/ssh-askpass")
    };
    for (const QString& candidate : candidates) {
        if (QFile::exists(candidate)) return candidate;
    }
    return QString();
}
constexpr int kUpdatesTimeoutMs = 30000;
constexpr int kNetworkTimeoutMs = 10000;
constexpr int kMaxSearchResults = 100;

QVariantMap itemFromRpcObject(const QJsonObject& object) {
    const QString name = object[QStringLiteral("Name")].toString();

    QVariantMap item;
    item[QStringLiteral("id")] = name;
    item[QStringLiteral("name")] = name;
    item[QStringLiteral("version")] = object[QStringLiteral("Version")].toString();
    item[QStringLiteral("summary")] = object[QStringLiteral("Description")].toString();
    item[QStringLiteral("backend")] = QStringLiteral("AUR");
    item[QStringLiteral("repository")] = QStringLiteral("aur");
    item[QStringLiteral("scope")] = QStringLiteral("system");
    item[QStringLiteral("icon")] = name;
    item[QStringLiteral("votes")] = object[QStringLiteral("NumVotes")].toInt();
    item[QStringLiteral("popularity")] = object[QStringLiteral("Popularity")].toDouble();
    item[QStringLiteral("outOfDate")] = !object[QStringLiteral("OutOfDate")].isNull();
    item[QStringLiteral("orphaned")] = object[QStringLiteral("Maintainer")].isNull();

    const QString maintainer = object[QStringLiteral("Maintainer")].toString();
    if (!maintainer.isEmpty()) item[QStringLiteral("developer")] = maintainer;

    return item;
}
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
            const QJsonArray entries = doc.object()[QStringLiteral("results")].toArray();
            QList<QVariantMap> items;
            items.reserve(entries.size());
            for (const QJsonValue& value : entries) {
                items.append(itemFromRpcObject(value.toObject()));
            }

            const QString needle = q.toLower();
            std::stable_sort(items.begin(), items.end(), [&needle](const QVariantMap& left, const QVariantMap& right) {
                const bool leftExact = left.value(QStringLiteral("name")).toString().toLower() == needle;
                const bool rightExact = right.value(QStringLiteral("name")).toString().toLower() == needle;
                if (leftExact != rightExact) return leftExact;
                return left.value(QStringLiteral("popularity")).toDouble() > right.value(QStringLiteral("popularity")).toDouble();
            });

            for (const QVariantMap& item : items) {
                if (results.size() >= kMaxSearchResults) break;
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
                const QJsonObject obj = arr.first().toObject();
                const QVariantMap item = itemFromRpcObject(obj);
                for (auto it = item.constBegin(); it != item.constEnd(); ++it) {
                    map[it.key()] = it.value();
                }

                map[QStringLiteral("description")] = obj[QStringLiteral("Description")].toString();
                map[QStringLiteral("homepage")] = obj[QStringLiteral("URL")].toString();
                map[QStringLiteral("aurPage")] = QStringLiteral("https://aur.archlinux.org/packages/") + packageId;

                if (obj[QStringLiteral("License")].isArray()) {
                    QStringList licenses;
                    for (const QJsonValue& license : obj[QStringLiteral("License")].toArray()) {
                        licenses.append(license.toString());
                    }
                    map[QStringLiteral("license")] = licenses.join(QStringLiteral(", "));
                }
            }
        }
    }
    reply->deleteLater();
    return map;
}

AurPlugin::HelperInvocation AurPlugin::buildHelperInvocation(const QStringList& operation) const {
    HelperInvocation invocation;
    invocation.arguments = operation;

    const QString helper = resolveHelper();
    if (helper.isEmpty()) {
        invocation.reason = QStringLiteral("No AUR helper found, install paru or yay");
        return invocation;
    }

    if (sudoIsAuthorised()) {
        invocation.usable = true;
        return invocation;
    }

    const bool supportsSudoFlags = helper == QStringLiteral("paru") || helper == QStringLiteral("yay");
    if (!supportsSudoFlags) {
        invocation.reason = QStringLiteral("%1 needs a password and cannot be asked for one here, run `sudo -v` first").arg(helper);
        return invocation;
    }

    const QString askpass = findAskpass();
    if (askpass.isEmpty()) {
        invocation.reason = QStringLiteral("%1 needs a password: run `sudo -v` in a terminal first, or install an askpass helper such as ksshaskpass").arg(helper);
        return invocation;
    }

    invocation.arguments << QStringLiteral("--sudoflags") << QStringLiteral("-A");
    invocation.environment.insert(QStringLiteral("SUDO_ASKPASS"), askpass);
    invocation.usable = true;
    return invocation;
}

bool AurPlugin::runHelperOperation(const QStringList& operation, ProgressCallback progressCb) {
    const QString helper = resolveHelper();
    const HelperInvocation invocation = buildHelperInvocation(operation);
    if (!invocation.usable) {
        if (progressCb) progressCb(0, invocation.reason);
        return false;
    }

    const astra::ProcessResult result = astra::runProcessStreaming(helper, invocation.arguments, [&progressCb](const QString& line) {
        if (progressCb) progressCb(50, line);
    }, 0, invocation.environment);

    if (progressCb && !result.succeeded() && result.started) {
        progressCb(0, QStringLiteral("%1 exited with status %2").arg(helper, QString::number(result.exitCode)));
    }
    return result.succeeded();
}

bool AurPlugin::install(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    Q_UNUSED(options);
    return runHelperOperation({QStringLiteral("-S"), QStringLiteral("--noconfirm"), packageId}, progressCb);
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
    return runHelperOperation({QStringLiteral("-Syu"), QStringLiteral("--noconfirm"), packageId}, progressCb);
}

bool AurPlugin::launch(const QString& packageId) {
    return QProcess::startDetached(packageId);
}
