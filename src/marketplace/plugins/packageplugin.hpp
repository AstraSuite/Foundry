#pragma once

#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QObject>
#include <functional>

class IPackagePlugin : public QObject {
    Q_OBJECT
public:
    using ProgressCallback = std::function<void(int progress, const QString& status)>;

    explicit IPackagePlugin(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IPackagePlugin() = default;

    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QString icon() const = 0;

    virtual bool isAvailable() const = 0;
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;

    virtual QVariantList search(const QString& query, const QVariantMap& options = {}) = 0;
    virtual QVariantList getInstalled() = 0;
    virtual QVariantList getUpdates() = 0;
    virtual QVariantMap getDetails(const QString& packageId) = 0;

    virtual bool install(const QString& packageId, const QVariantMap& options = {}, ProgressCallback progressCb = nullptr) = 0;
    virtual bool uninstall(const QString& packageId, const QVariantMap& options = {}, ProgressCallback progressCb = nullptr) = 0;

    virtual bool update(const QString& packageId, const QVariantMap& options = {}, ProgressCallback progressCb = nullptr) {
        return install(packageId, options, progressCb);
    }

    virtual bool launch(const QString& packageId) = 0;

    virtual QList<QVariantMap> getInstallSources(const QString& packageId) {
        Q_UNUSED(packageId);
        return {};
    }

signals:
    void availabilityChanged();
    void enabledChanged();
    void statusMessage(const QString& message);
    void progress(const QString& packageId, int percent, const QString& message);
};
