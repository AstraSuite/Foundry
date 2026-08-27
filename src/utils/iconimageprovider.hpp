#pragma once

#include <QHash>
#include <QMutex>
#include <QQuickImageProvider>
#include <QStringList>

class IconImageProvider : public QQuickImageProvider {
public:
    IconImageProvider();

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

    static QString resolvePath(const QString& name);
    static bool hasIcon(const QString& name);

private:
    static QString searchThemeDirectories(const QString& name);
    static QString searchDesktopEntries(const QString& name);
    static void loadDesktopEntries();

    static QStringList s_themeDirectories;
    static QHash<QString, QString> s_resolvedPaths;
    static QHash<QString, QString> s_desktopIcons;
    static bool s_desktopEntriesLoaded;
    static QMutex s_mutex;
};
