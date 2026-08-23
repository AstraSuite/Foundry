#pragma once

#include "packageplugin.hpp"
#include <QSet>
#include <QSettings>

class PacmanPlugin : public IPackagePlugin {
    Q_OBJECT
public:
    explicit PacmanPlugin(QObject* parent = nullptr);

    QString id() const override { return QStringLiteral("Pacman"); }
    QString name() const override { return QStringLiteral("Pacman"); }
    QString description() const override { return QStringLiteral("Native Arch Linux system packages"); }
    QString icon() const override { return QStringLiteral("package_2"); }

    bool isAvailable() const override;
    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool enabled) override;

    QVariantList search(const QString& query, const QVariantMap& options = {}) override;
    QVariantList getInstalled() override;
    QVariantList getUpdates() override;
    QVariantMap getDetails(const QString& packageId) override;

    bool install(const QString& packageId, const QVariantMap& options = {}, ProgressCallback progressCb = nullptr) override;
    bool uninstall(const QString& packageId, const QVariantMap& options = {}, ProgressCallback progressCb = nullptr) override;
    bool launch(const QString& packageId) override;

    static QVariantList parseSearchOutput(const QString& output);
    static QSet<QString> parseForeignOutput(const QString& output);
    static QVariantList parseInstalledOutput(const QString& output, const QSet<QString>& foreign);
    static QVariantList parseUpdatesOutput(const QString& output);
    static QVariantMap parseInfoOutput(const QString& output, const QString& packageId);

private:
    bool m_enabled{true};
    QSettings m_settings{QStringLiteral("AstraMarket"), QStringLiteral("Plugins")};
};
