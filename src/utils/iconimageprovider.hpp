#pragma once

#include <QHash>
#include <QMutex>
#include <QQuickImageProvider>
#include <QStringList>

class IconImageProvider : public QQuickImageProvider {
public:
    IconImageProvider();

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

private:
    QString resolvePath(const QString& name);
    QString searchThemeDirectories(const QString& name) const;
    QString searchDesktopEntries(const QString& name);
    void loadDesktopEntries();

    const QStringList m_themeDirectories;
    QHash<QString, QString> m_resolvedPaths;
    QHash<QString, QString> m_desktopIcons;
    bool m_desktopEntriesLoaded{false};
    QMutex m_mutex;
};
