#pragma once

#include "packageplugin.hpp"
#include <QMutex>
#include <QSettings>

class AppImagePlugin : public IPackagePlugin {
    Q_OBJECT
public:
    explicit AppImagePlugin(QObject* parent = nullptr);

    QString id() const override { return QStringLiteral("AppImage"); }
    QString name() const override { return QStringLiteral("AppImage"); }
    QString description() const override { return QStringLiteral("Standalone portable Linux applications"); }
    QString icon() const override { return QStringLiteral("extension"); }

    bool isAvailable() const override { return true; }
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

    QVariantList getCatalog();

    static QVariantList parseCatalog(const QByteArray& payload);
    static QString pickReleaseAsset(const QByteArray& payload, QString* fileName);

private:
    QVariantMap catalogEntry(const QString& packageId);
    QString resolveDownloadUrl(const QString& repository, QString* fileName);
    QString downloadRelease(const QString& url, const QString& fileName, ProgressCallback progressCb);

    bool m_enabled{true};
    bool m_catalogLoaded{false};
    QVariantList m_catalog;
    QMutex m_catalogMutex;
    QSettings m_settings{QStringLiteral("astra-foundry"), QStringLiteral("Plugins")};
};
