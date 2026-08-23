#include "scriptableplugin.hpp"
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>

namespace {
constexpr int kOperationTimeoutMs = 3600000;
}

ScriptablePlugin::ScriptablePlugin(const QString& manifestPath, QObject* parent)
    : IPackagePlugin(parent), m_manifestPath(manifestPath) {
    QFileInfo fi(manifestPath);
    m_baseDir = fi.absolutePath();

    QFile file(manifestPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            m_id = obj.value(QStringLiteral("id")).toString(fi.dir().dirName());
            m_name = obj.value(QStringLiteral("name")).toString(m_id);
            m_description = obj.value(QStringLiteral("description")).toString();
            m_icon = obj.value(QStringLiteral("icon")).toString(QStringLiteral("extension"));

            QJsonObject cmds = obj.value(QStringLiteral("commands")).toObject();
            m_searchCmd = cmds.value(QStringLiteral("search")).toString();
            m_installCmd = cmds.value(QStringLiteral("install")).toString();
            m_uninstallCmd = cmds.value(QStringLiteral("uninstall")).toString();
            m_detailsCmd = cmds.value(QStringLiteral("details")).toString();
            m_listCmd = cmds.value(QStringLiteral("list")).toString();
            m_updatesCmd = cmds.value(QStringLiteral("updates")).toString();
            m_launchCmd = cmds.value(QStringLiteral("launch")).toString();

            QString checkBin = obj.value(QStringLiteral("requiredBinary")).toString();
            if (!checkBin.isEmpty()) {
                m_isAvailable = checkBin.contains(QLatin1Char('/'))
                    ? QFile::exists(checkBin)
                    : !QStandardPaths::findExecutable(checkBin).isEmpty();
            }
        }
    }

    m_enabled = m_settings.value(m_id + QStringLiteral("/enabled"), true).toBool();
}

void ScriptablePlugin::setEnabled(bool enabled) {
    if (m_enabled != enabled) {
        m_enabled = enabled;
        m_settings.setValue(m_id + QStringLiteral("/enabled"), enabled);
        emit enabledChanged();
    }
}

QStringList ScriptablePlugin::buildShellArguments(const QString& cmdTemplate, const QMap<QString, QString>& vars) {
    static const QRegularExpression placeholder(QStringLiteral(R"(\$\{([A-Za-z_][A-Za-z0-9_]*)\})"));

    QString script;
    QStringList positional;
    QMap<QString, int> indices;
    qsizetype cursor = 0;

    QRegularExpressionMatchIterator it = placeholder.globalMatch(cmdTemplate);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        script += cmdTemplate.mid(cursor, match.capturedStart() - cursor);

        const QString name = match.captured(1);
        int index = indices.value(name, 0);
        if (index == 0) {
            positional.append(vars.value(name));
            index = positional.size();
            indices.insert(name, index);
        }

        script += QStringLiteral("${") + QString::number(index) + QStringLiteral("}");
        cursor = match.capturedEnd();
    }
    script += cmdTemplate.mid(cursor);

    QStringList args{QStringLiteral("-c"), script, QStringLiteral("astra-plugin")};
    args.append(positional);
    return args;
}

ScriptablePlugin::ScriptResult ScriptablePlugin::runCommand(const QString& cmdTemplate, const QMap<QString, QString>& vars, ProgressCallback progressCb, int timeoutMs) {
    ScriptResult result;
    if (cmdTemplate.isEmpty()) return result;

    QProcess proc;
    proc.setWorkingDirectory(m_baseDir);
    proc.start(QStringLiteral("/bin/sh"), buildShellArguments(cmdTemplate, vars));
    if (!proc.waitForStarted(5000)) {
        proc.kill();
        proc.waitForFinished(1000);
        return result;
    }

    QElapsedTimer elapsed;
    elapsed.start();
    QByteArray pending;

    auto drain = [&](bool flush) {
        const QByteArray chunk = proc.readAllStandardOutput();
        if (!chunk.isEmpty()) {
            result.output.append(chunk);
            if (progressCb) pending.append(chunk);
        }
        result.error.append(proc.readAllStandardError());

        if (!progressCb) return;
        while (true) {
            const qsizetype breakAt = pending.indexOf('\n');
            if (breakAt < 0) break;
            const QString line = QString::fromUtf8(pending.left(breakAt)).trimmed();
            pending.remove(0, breakAt + 1);
            if (!line.isEmpty()) progressCb(50, line);
        }
        if (flush && !pending.isEmpty()) {
            const QString line = QString::fromUtf8(pending).trimmed();
            pending.clear();
            if (!line.isEmpty()) progressCb(50, line);
        }
    };

    while (proc.state() == QProcess::Running) {
        proc.waitForReadyRead(200);
        drain(false);

        if (timeoutMs > 0 && elapsed.hasExpired(timeoutMs)) {
            result.timedOut = true;
            proc.kill();
            proc.waitForFinished(1000);
            break;
        }
    }

    proc.waitForFinished(1000);
    drain(true);

    result.exitCode = result.timedOut ? -1 : proc.exitCode();
    return result;
}

QVariantList ScriptablePlugin::search(const QString& query, const QVariantMap& options) {
    Q_UNUSED(options);
    QVariantList results;
    if (!m_enabled || m_searchCmd.isEmpty()) return results;

    QMap<QString, QString> vars;
    vars[QStringLiteral("QUERY")] = query;
    const ScriptResult result = runCommand(m_searchCmd, vars);

    QJsonDocument doc = QJsonDocument::fromJson(result.output);
    if (doc.isArray()) {
        for (const QJsonValue& val : doc.array()) {
            if (val.isObject()) {
                QVariantMap item = val.toObject().toVariantMap();
                if (!item.contains(QStringLiteral("backend"))) item[QStringLiteral("backend")] = m_name;
                if (!item.contains(QStringLiteral("icon"))) item[QStringLiteral("icon")] = m_icon;
                results.append(item);
            }
        }
    }
    return results;
}

QVariantList ScriptablePlugin::getInstalled() {
    QVariantList results;
    if (!m_enabled || m_listCmd.isEmpty()) return results;

    const ScriptResult result = runCommand(m_listCmd, {});
    QJsonDocument doc = QJsonDocument::fromJson(result.output);
    if (doc.isArray()) {
        for (const QJsonValue& val : doc.array()) {
            if (val.isObject()) {
                QVariantMap item = val.toObject().toVariantMap();
                if (!item.contains(QStringLiteral("backend"))) item[QStringLiteral("backend")] = m_name;
                item[QStringLiteral("isInstalled")] = true;
                results.append(item);
            }
        }
    }
    return results;
}

QVariantList ScriptablePlugin::getUpdates() {
    QVariantList results;
    if (!m_enabled || m_updatesCmd.isEmpty()) return results;

    const ScriptResult result = runCommand(m_updatesCmd, {});
    QJsonDocument doc = QJsonDocument::fromJson(result.output);
    if (doc.isArray()) {
        for (const QJsonValue& val : doc.array()) {
            if (val.isObject()) {
                QVariantMap item = val.toObject().toVariantMap();
                if (!item.contains(QStringLiteral("backend"))) item[QStringLiteral("backend")] = m_name;
                results.append(item);
            }
        }
    }
    return results;
}

QVariantMap ScriptablePlugin::getDetails(const QString& packageId) {
    QVariantMap map;
    map[QStringLiteral("id")] = packageId;
    map[QStringLiteral("name")] = packageId;
    map[QStringLiteral("backend")] = m_name;

    if (!m_enabled || m_detailsCmd.isEmpty()) return map;

    QMap<QString, QString> vars;
    vars[QStringLiteral("ID")] = packageId;
    const ScriptResult result = runCommand(m_detailsCmd, vars);
    QJsonDocument doc = QJsonDocument::fromJson(result.output);
    if (doc.isObject()) {
        QVariantMap parsed = doc.object().toVariantMap();
        for (auto it = parsed.begin(); it != parsed.end(); ++it) {
            map[it.key()] = it.value();
        }
    }
    return map;
}

bool ScriptablePlugin::install(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    if (!m_enabled || m_installCmd.isEmpty()) return false;
    QMap<QString, QString> vars;
    vars[QStringLiteral("ID")] = packageId;
    for (auto it = options.begin(); it != options.end(); ++it) {
        vars[it.key().toUpper()] = it.value().toString();
    }
    return runCommand(m_installCmd, vars, progressCb, kOperationTimeoutMs).exitCode == 0;
}

bool ScriptablePlugin::uninstall(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    if (!m_enabled || m_uninstallCmd.isEmpty()) return false;
    QMap<QString, QString> vars;
    vars[QStringLiteral("ID")] = packageId;
    for (auto it = options.begin(); it != options.end(); ++it) {
        vars[it.key().toUpper()] = it.value().toString();
    }
    return runCommand(m_uninstallCmd, vars, progressCb, kOperationTimeoutMs).exitCode == 0;
}

bool ScriptablePlugin::launch(const QString& packageId) {
    if (!m_enabled || m_launchCmd.isEmpty()) return false;
    QMap<QString, QString> vars;
    vars[QStringLiteral("ID")] = packageId;
    return QProcess::startDetached(QStringLiteral("/bin/sh"), buildShellArguments(m_launchCmd, vars), m_baseDir);
}

QList<ScriptablePlugin*> ScriptablePlugin::loadFromDirectories(const QStringList& directories, QObject* parent) {
    QList<ScriptablePlugin*> plugins;
    for (const QString& dirPath : directories) {
        QDir dir(dirPath);
        if (!dir.exists()) continue;

        QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString& sub : subdirs) {
            QString manifest = dir.filePath(sub + QStringLiteral("/plugin.json"));
            if (QFile::exists(manifest)) {
                plugins.append(new ScriptablePlugin(manifest, parent));
            }
        }
    }
    return plugins;
}
