#pragma once

#include "packageplugin.hpp"
#include <QJsonObject>
#include <QSet>
#include <QSettings>

class FlatpakPlugin : public IPackagePlugin {
    Q_OBJECT
public:
    explicit FlatpakPlugin(QObject* parent = nullptr);

    QString id() const override { return QStringLiteral("Flatpak"); }
    QString name() const override { return QStringLiteral("Flatpak"); }
    QString description() const override { return QStringLiteral("Sandboxed applications from Flathub"); }
    QString icon() const override { return QStringLiteral("deployed_code"); }

    bool isAvailable() const override;
    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool enabled) override;

    QVariantList search(const QString& query, const QVariantMap& options = {}) override;
    QVariantList getInstalled() override;
    QVariantList getUpdates() override;
    QVariantMap getDetails(const QString& packageId) override;

    bool install(const QString& packageId, const QVariantMap& options = {}, ProgressCallback progressCb = nullptr) override;
    bool uninstall(const QString& packageId, const QVariantMap& options = {}, ProgressCallback progressCb = nullptr) override;
    bool update(const QString& packageId, const QVariantMap& options = {}, ProgressCallback progressCb = nullptr) override;
    bool launch(const QString& packageId) override;

    QList<QVariantMap> getInstallSources(const QString& packageId) override;

    QVariantList getCollection(const QString& collection, int limit);

    static QVariantList permissionEntries(const QJsonObject& permissions);
    static QVariantList parseLocalPermissions(const QString& output);
    static QString formatBytes(qint64 bytes);

    static QString scopeArgument(const QString& scope);
    static QVariantList parseCollectionHits(const QByteArray& payload, QSet<QString>& seen);
    static QVariantList parseSearchOutput(const QString& output, QSet<QString>& seen);
    static QVariantList parseInstalledOutput(const QString& output, const QString& scope);
    static QVariantList parseUpdatesOutput(const QString& output);

private:
    QString resolveScope(const QString& packageId, const QVariantMap& options);

    bool m_enabled{true};
    QSettings m_settings{QStringLiteral("AstraMarket"), QStringLiteral("Plugins")};
};
