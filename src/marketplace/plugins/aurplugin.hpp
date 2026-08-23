#pragma once

#include "packageplugin.hpp"
#include <QMap>
#include <QSettings>

class AurPlugin : public IPackagePlugin {
    Q_OBJECT
public:
    explicit AurPlugin(QObject* parent = nullptr);

    QString id() const override { return QStringLiteral("AUR"); }
    QString name() const override { return QStringLiteral("AUR"); }
    QString description() const override { return QStringLiteral("Arch User Repository community packages"); }
    QString icon() const override { return QStringLiteral("folder_zip"); }

    bool isAvailable() const override;
    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool enabled) override;

    QString aurHelper() const { return m_aurHelper; }
    void setAurHelper(const QString& helper);

    QVariantList search(const QString& query, const QVariantMap& options = {}) override;
    QVariantList getInstalled() override;
    QVariantList getUpdates() override;
    QVariantMap getDetails(const QString& packageId) override;

    bool install(const QString& packageId, const QVariantMap& options = {}, ProgressCallback progressCb = nullptr) override;
    bool uninstall(const QString& packageId, const QVariantMap& options = {}, ProgressCallback progressCb = nullptr) override;
    bool update(const QString& packageId, const QVariantMap& options = {}, ProgressCallback progressCb = nullptr) override;
    bool launch(const QString& packageId) override;

    QString getBuildScript(const QString& packageId);

    static QVariantList parseForeignOutput(const QString& output);
    static QVariantList parseUpdatesOutput(const QString& output);

private:
    struct HelperInvocation {
        bool usable{false};
        QString reason;
        QStringList arguments;
        QMap<QString, QString> environment;
    };

    HelperInvocation buildHelperInvocation(const QStringList& operation) const;
    bool runHelperOperation(const QStringList& operation, ProgressCallback progressCb);

    bool m_enabled{true};
    QString m_aurHelper{QStringLiteral("auto")};
    QSettings m_settings{QStringLiteral("AstraMarket"), QStringLiteral("Plugins")};

    QString resolveHelper() const;
};
