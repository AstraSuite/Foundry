#include "pacmanplugin.hpp"
#include "pluginprocess.hpp"

#include <QFile>
#include <QProcess>
#include <QStandardPaths>

namespace {
constexpr int kQueryTimeoutMs = 15000;
constexpr int kUpdatesTimeoutMs = 30000;
}

PacmanPlugin::PacmanPlugin(QObject* parent) : IPackagePlugin(parent) {
    m_enabled = m_settings.value(QStringLiteral("Pacman/enabled"), true).toBool();
}

bool PacmanPlugin::isAvailable() const {
    return !QStandardPaths::findExecutable(QStringLiteral("pacman")).isEmpty();
}

void PacmanPlugin::setEnabled(bool enabled) {
    if (m_enabled != enabled) {
        m_enabled = enabled;
        m_settings.setValue(QStringLiteral("Pacman/enabled"), enabled);
        emit enabledChanged();
    }
}

QVariantList PacmanPlugin::parseSearchOutput(const QString& output) {
    QVariantList results;
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    QSet<QString> seen;

    for (qsizetype i = 0; i < lines.size(); ++i) {
        const QString& header = lines.at(i);
        if (header.startsWith(QLatin1Char(' ')) || header.startsWith(QLatin1Char('\t'))) continue;

        QString description;
        if (i + 1 < lines.size()) {
            const QString& next = lines.at(i + 1);
            if (next.startsWith(QLatin1Char(' ')) || next.startsWith(QLatin1Char('\t'))) {
                description = next.trimmed();
                ++i;
            }
        }

        const QString fullName = header.split(QLatin1Char(' '), Qt::SkipEmptyParts).value(0);
        if (fullName.isEmpty()) continue;

        const QString repository = fullName.contains(QLatin1Char('/')) ? fullName.section(QLatin1Char('/'), 0, 0) : QStringLiteral("extra");
        const QString packageName = fullName.contains(QLatin1Char('/')) ? fullName.section(QLatin1Char('/'), 1) : fullName;
        if (packageName.isEmpty() || seen.contains(packageName)) continue;
        seen.insert(packageName);

        const QStringList headerParts = header.split(QLatin1Char(' '), Qt::SkipEmptyParts);

        QVariantMap item;
        item[QStringLiteral("id")] = packageName;
        item[QStringLiteral("name")] = packageName;
        item[QStringLiteral("version")] = headerParts.value(1);
        item[QStringLiteral("summary")] = description;
        item[QStringLiteral("backend")] = QStringLiteral("Pacman");
        item[QStringLiteral("repository")] = repository;
        item[QStringLiteral("scope")] = QStringLiteral("system");
        item[QStringLiteral("icon")] = packageName;
        item[QStringLiteral("isInstalled")] = header.contains(QStringLiteral("[installed"));
        results.append(item);
    }
    return results;
}

QSet<QString> PacmanPlugin::parseForeignOutput(const QString& output) {
    QSet<QString> packages;
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        const QString name = line.split(QLatin1Char(' '), Qt::SkipEmptyParts).value(0);
        if (!name.isEmpty()) packages.insert(name);
    }
    return packages;
}

QVariantList PacmanPlugin::parseInstalledOutput(const QString& output, const QSet<QString>& foreign) {
    QVariantList results;
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        const QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (parts.size() < 2) continue;

        const QString packageId = parts.value(0);
        if (foreign.contains(packageId)) continue;

        QVariantMap item;
        item[QStringLiteral("id")] = packageId;
        item[QStringLiteral("name")] = packageId;
        item[QStringLiteral("version")] = parts.value(1);
        item[QStringLiteral("backend")] = QStringLiteral("Pacman");
        item[QStringLiteral("repository")] = QStringLiteral("extra");
        item[QStringLiteral("scope")] = QStringLiteral("system");
        item[QStringLiteral("icon")] = packageId;
        item[QStringLiteral("isInstalled")] = true;
        results.append(item);
    }
    return results;
}

QVariantList PacmanPlugin::parseUpdatesOutput(const QString& output) {
    QVariantList results;
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    for (const QString& line : lines) {
        const QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (parts.size() < 4) continue;

        QVariantMap item;
        item[QStringLiteral("id")] = parts.value(0);
        item[QStringLiteral("name")] = parts.value(0);
        item[QStringLiteral("version")] = parts.value(1) + QStringLiteral(" -> ") + parts.value(3);
        item[QStringLiteral("backend")] = QStringLiteral("Pacman");
        item[QStringLiteral("scope")] = QStringLiteral("system");
        item[QStringLiteral("icon")] = parts.value(0);
        results.append(item);
    }
    return results;
}

QVariantMap PacmanPlugin::parseInfoOutput(const QString& output, const QString& packageId) {
    QVariantMap map;
    map[QStringLiteral("id")] = packageId;
    map[QStringLiteral("name")] = packageId;
    map[QStringLiteral("backend")] = QStringLiteral("Pacman");

    QMap<QString, QString> fields;
    QString currentField;

    const QStringList lines = output.split(QLatin1Char('\n'));
    for (const QString& line : lines) {
        if (line.trimmed().isEmpty()) continue;

        const qsizetype separator = line.indexOf(QLatin1String(" : "));
        const bool continuation = line.startsWith(QLatin1Char(' ')) || line.startsWith(QLatin1Char('\t'));

        if (!continuation && separator > 0) {
            currentField = line.left(separator).trimmed();
            fields.insert(currentField, line.mid(separator + 3).trimmed());
        } else if (!currentField.isEmpty()) {
            fields[currentField] += QLatin1Char(' ') + line.trimmed();
        }
    }

    const auto assign = [&](const QString& field, const QString& key) {
        const QString value = fields.value(field);
        if (!value.isEmpty() && value != QLatin1String("None")) map[key] = value;
    };

    assign(QStringLiteral("Description"), QStringLiteral("summary"));
    assign(QStringLiteral("Description"), QStringLiteral("description"));
    assign(QStringLiteral("Version"), QStringLiteral("version"));
    assign(QStringLiteral("URL"), QStringLiteral("homepage"));
    assign(QStringLiteral("Licenses"), QStringLiteral("license"));
    assign(QStringLiteral("Packager"), QStringLiteral("developer"));
    assign(QStringLiteral("Repository"), QStringLiteral("repository"));
    assign(QStringLiteral("Installed Size"), QStringLiteral("size"));
    assign(QStringLiteral("Download Size"), QStringLiteral("downloadSize"));

    return map;
}

QVariantList PacmanPlugin::search(const QString& query, const QVariantMap& options) {
    Q_UNUSED(options);
    if (!isAvailable() || !m_enabled) return {};

    const QString trimmed = query.trimmed();
    if (trimmed.isEmpty()) return {};

    const astra::ProcessResult result = astra::runProcess(QStringLiteral("pacman"), {QStringLiteral("-Ss"), trimmed.toLower()}, kQueryTimeoutMs);
    return parseSearchOutput(result.output);
}

QVariantList PacmanPlugin::getInstalled() {
    if (!isAvailable() || !m_enabled) return {};

    const astra::ProcessResult foreign = astra::runProcess(QStringLiteral("pacman"), {QStringLiteral("-Qm")}, kQueryTimeoutMs);
    const astra::ProcessResult explicitly = astra::runProcess(QStringLiteral("pacman"), {QStringLiteral("-Qe")}, kQueryTimeoutMs);
    return parseInstalledOutput(explicitly.output, parseForeignOutput(foreign.output));
}

QVariantList PacmanPlugin::getUpdates() {
    if (!isAvailable() || !m_enabled) return {};
    if (QStandardPaths::findExecutable(QStringLiteral("checkupdates")).isEmpty()) return {};

    const astra::ProcessResult result = astra::runProcess(QStringLiteral("checkupdates"), {}, kUpdatesTimeoutMs);
    return parseUpdatesOutput(result.output);
}

QVariantMap PacmanPlugin::getDetails(const QString& packageId) {
    astra::ProcessResult result = astra::runProcess(QStringLiteral("pacman"), {QStringLiteral("-Si"), packageId}, kQueryTimeoutMs);
    if (!result.succeeded() || result.output.trimmed().isEmpty()) {
        result = astra::runProcess(QStringLiteral("pacman"), {QStringLiteral("-Qi"), packageId}, kQueryTimeoutMs);
    }
    return parseInfoOutput(result.output, packageId);
}

bool PacmanPlugin::install(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    Q_UNUSED(options);
    if (!isAvailable()) return false;

    const astra::ProcessResult result = astra::runProcessStreaming(
        QStringLiteral("pkexec"),
        {QStringLiteral("pacman"), QStringLiteral("-S"), QStringLiteral("--noconfirm"), packageId},
        [&progressCb](const QString& line) {
            if (progressCb) progressCb(50, line);
        });

    if (progressCb && !result.succeeded() && !result.started) {
        progressCb(0, QStringLiteral("Could not start pkexec"));
    }
    return result.succeeded();
}

bool PacmanPlugin::uninstall(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    Q_UNUSED(options);
    if (!isAvailable()) return false;

    const astra::ProcessResult result = astra::runProcessStreaming(
        QStringLiteral("pkexec"),
        {QStringLiteral("pacman"), QStringLiteral("-Rns"), QStringLiteral("--noconfirm"), packageId},
        [&progressCb](const QString& line) {
            if (progressCb) progressCb(50, line);
        });

    if (progressCb && !result.succeeded() && !result.started) {
        progressCb(0, QStringLiteral("Could not start pkexec"));
    }
    return result.succeeded();
}

bool PacmanPlugin::launch(const QString& packageId) {
    if (!isAvailable()) return false;

    const astra::ProcessResult files = astra::runProcess(QStringLiteral("pacman"), {QStringLiteral("-Qlq"), packageId}, kQueryTimeoutMs);
    const QStringList lines = files.output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    QString fallback;
    for (const QString& line : lines) {
        const QString path = line.trimmed();
        if (!path.startsWith(QLatin1String("/usr/bin/")) || path.endsWith(QLatin1Char('/'))) continue;

        const QString binary = path.mid(9);
        if (binary.isEmpty()) continue;
        if (binary.compare(packageId, Qt::CaseInsensitive) == 0) return QProcess::startDetached(path);
        if (fallback.isEmpty()) fallback = path;
    }

    if (!fallback.isEmpty()) return QProcess::startDetached(fallback);
    return QProcess::startDetached(packageId);
}
