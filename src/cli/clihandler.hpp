#pragma once

#include <QStringList>
#include "../marketplace/packagemanager.hpp"

class CliHandler {
public:
    static int run(int argc, char* argv[], PackageManager& pm);

private:
    struct Options {
        QString source;
        QString scope;
        bool json{false};
        QStringList positional;
    };

    static Options parseOptions(const QStringList& args);
    static IPackagePlugin* resolveSource(const QString& packageId, const Options& options, PackageManager& pm, bool installedOnly, bool requireUnique);

    static void printHelp();
    static void printVersion();
    static int handleSearch(const Options& options, PackageManager& pm);
    static int handleInstall(const Options& options, PackageManager& pm);
    static int handleUninstall(const Options& options, PackageManager& pm);
    static int handleList(const Options& options, PackageManager& pm);
    static int handleUpdate(const Options& options, PackageManager& pm);
    static int handleUpgrade(const Options& options, PackageManager& pm);
    static int handleInfo(const Options& options, PackageManager& pm);
    static int handleSources(const Options& options, PackageManager& pm);
    static int installAppImageFile(const QString& path);
};
