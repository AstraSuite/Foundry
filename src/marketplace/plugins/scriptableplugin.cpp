#include "scriptableplugin.hpp"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>

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
                m_isAvailable = QFile::exists(checkBin) || QFile::exists(QStringLiteral("/usr/bin/") + checkBin);
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

QByteArray ScriptablePlugin::runCommand(const QString& cmdTemplate, const QMap<QString, QString>& vars, ProgressCallback progressCb) {
    if (cmdTemplate.isEmpty()) return QByteArray();

    QString cmd = cmdTemplate;
    for (auto it = vars.begin(); it != vars.end(); ++it) {
        cmd.replace(QLatin1String("${") + it.key() + QLatin1String("}"), it.value());
    }

    QProcess proc;
    proc.setWorkingDirectory(m_baseDir);

    if (progressCb) {
        QObject::connect(&proc, &QProcess::readyReadStandardOutput, [&proc, &progressCb]() {
            QString line = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
            progressCb(50, line);
        });
    }

    proc.start(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), cmd});
    proc.waitForFinished(10000);
    return proc.readAllStandardOutput();
}

QVariantList ScriptablePlugin::search(const QString& query, const QVariantMap& options) {
    Q_UNUSED(options);
    QVariantList results;
    if (!m_enabled || m_searchCmd.isEmpty()) return results;

    QMap<QString, QString> vars;
    vars[QStringLiteral("QUERY")] = query;
    QByteArray out = runCommand(m_searchCmd, vars);

    QJsonDocument doc = QJsonDocument::fromJson(out);
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

    QByteArray out = runCommand(m_listCmd, {});
    QJsonDocument doc = QJsonDocument::fromJson(out);
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

    QByteArray out = runCommand(m_updatesCmd, {});
    QJsonDocument doc = QJsonDocument::fromJson(out);
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
    QByteArray out = runCommand(m_detailsCmd, vars);
    QJsonDocument doc = QJsonDocument::fromJson(out);
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
    runCommand(m_installCmd, vars, progressCb);
    return true;
}

bool ScriptablePlugin::uninstall(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    if (!m_enabled || m_uninstallCmd.isEmpty()) return false;
    QMap<QString, QString> vars;
    vars[QStringLiteral("ID")] = packageId;
    for (auto it = options.begin(); it != options.end(); ++it) {
        vars[it.key().toUpper()] = it.value().toString();
    }
    runCommand(m_uninstallCmd, vars, progressCb);
    return true;
}

bool ScriptablePlugin::launch(const QString& packageId) {
    if (!m_enabled || m_launchCmd.isEmpty()) return false;
    QMap<QString, QString> vars;
    vars[QStringLiteral("ID")] = packageId;
    runCommand(m_launchCmd, vars);
    return true;
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
