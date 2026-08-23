#pragma once

#include "packageplugin.hpp"
#include <QByteArray>
#include <QJsonObject>
#include <QMap>
#include <QSettings>

class ScriptablePlugin : public IPackagePlugin {
    Q_OBJECT
public:
    explicit ScriptablePlugin(const QString& manifestPath, QObject* parent = nullptr);

    QString id() const override { return m_id; }
    QString name() const override { return m_name; }
    QString description() const override { return m_description; }
    QString icon() const override { return m_icon; }

    bool isAvailable() const override { return m_isAvailable; }
    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool enabled) override;

    QVariantList search(const QString& query, const QVariantMap& options = {}) override;
    QVariantList getInstalled() override;
    QVariantList getUpdates() override;
    QVariantMap getDetails(const QString& packageId) override;

    bool install(const QString& packageId, const QVariantMap& options = {}, ProgressCallback progressCb = nullptr) override;
    bool uninstall(const QString& packageId, const QVariantMap& options = {}, ProgressCallback progressCb = nullptr) override;
    bool launch(const QString& packageId) override;

    static QList<ScriptablePlugin*> loadFromDirectories(const QStringList& directories, QObject* parent = nullptr);

private:
    QString m_manifestPath;
    QString m_baseDir;
    QString m_id;
    QString m_name;
    QString m_description;
    QString m_icon{QStringLiteral("extension")};
    bool m_isAvailable{true};
    bool m_enabled{true};

    QString m_searchCmd;
    QString m_installCmd;
    QString m_uninstallCmd;
    QString m_detailsCmd;
    QString m_listCmd;
    QString m_updatesCmd;
    QString m_launchCmd;

    QSettings m_settings{QStringLiteral("AstraMarket"), QStringLiteral("Plugins")};

    struct ScriptResult {
        int exitCode{-1};
        QByteArray output;
        QByteArray error;
        bool timedOut{false};
    };

    static QStringList buildShellArguments(const QString& cmdTemplate, const QMap<QString, QString>& vars);
    ScriptResult runCommand(const QString& cmdTemplate, const QMap<QString, QString>& vars, ProgressCallback progressCb = nullptr, int timeoutMs = 20000);
};
