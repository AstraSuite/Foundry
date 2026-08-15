#pragma once

#include <QStringList>
#include "../marketplace/packagemanager.hpp"

class CliHandler {
public:
    static int run(int argc, char* argv[], PackageManager& pm);

private:
    static void printHelp();
    static void printVersion();
    static int handleSearch(const QStringList& args, PackageManager& pm);
    static int handleInstall(const QStringList& args, PackageManager& pm);
    static int handleUninstall(const QStringList& args, PackageManager& pm);
    static int handleList(const QStringList& args, PackageManager& pm);
    static int handleUpdate(const QStringList& args, PackageManager& pm);
    static int handleInfo(const QStringList& args, PackageManager& pm);
    static int handleSources(const QStringList& args, PackageManager& pm);
};
