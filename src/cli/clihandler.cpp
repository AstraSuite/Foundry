#include "clihandler.hpp"
#include "marketplace/appimageinstaller.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>
#include <iomanip>
#include <iostream>
#include <unistd.h>

#ifndef ASTRA_VERSION
#define ASTRA_VERSION "1.1.0"
#endif

namespace {

bool g_colours = true;

const char* colour(const char* code) {
    return g_colours ? code : "";
}

const char* reset() { return colour("\033[0m"); }
const char* bold() { return colour("\033[1m"); }
const char* dim() { return colour("\033[2m"); }
const char* cyan() { return colour("\033[36m"); }
const char* green() { return colour("\033[32m"); }
const char* yellow() { return colour("\033[33m"); }
const char* red() { return colour("\033[31m"); }
const char* magenta() { return colour("\033[35m"); }

const char* backendColour(const QString& backend) {
    if (backend == QStringLiteral("Pacman")) return green();
    if (backend == QStringLiteral("AUR")) return magenta();
    if (backend == QStringLiteral("AppImage")) return yellow();
    return cyan();
}

std::string text(const QString& value) {
    return value.toStdString();
}

QString truncated(const QString& value, int width) {
    return value.length() > width ? value.left(width - 3) + QStringLiteral("...") : value;
}

void printJson(const QJsonDocument& document) {
    std::cout << document.toJson(QJsonDocument::Indented).toStdString();
}

QJsonArray toJsonArray(const QVariantList& items) {
    QJsonArray array;
    for (const QVariant& item : items) {
        array.append(QJsonObject::fromVariantMap(item.toMap()));
    }
    return array;
}

void reportProgress(int percent, const QString& status) {
    Q_UNUSED(percent);
    if (!status.isEmpty()) {
        std::cout << dim() << "  " << text(status) << reset() << "\n";
    }
}

bool looksLikeAppImage(const QString& value) {
    return value.endsWith(QLatin1String(".appimage"), Qt::CaseInsensitive) ||
           (value.startsWith(QLatin1String("file://")) && value.contains(QLatin1String("appimage"), Qt::CaseInsensitive));
}

}

void CliHandler::printVersion() {
    std::cout << bold() << cyan() << "astra" << reset() << " version " << green() << ASTRA_VERSION << reset()
              << " (AstraMarket Universal Package Manager)\n";
}

void CliHandler::printHelp() {
    std::cout << bold() << cyan() << "AstraMarket CLI" << reset() << " - Unified Linux package management\n\n"
              << bold() << "USAGE:" << reset() << "\n"
              << "  astra <COMMAND> [OPTIONS]\n\n"
              << bold() << "COMMANDS:" << reset() << "\n"
              << "  " << green() << "search" << reset() << " <query>          Search for packages across all or specific sources\n"
              << "  " << green() << "install" << reset() << " <pkg>           Install a package or a local .AppImage file\n"
              << "  " << green() << "remove" << reset() << " <pkg>            Uninstall a package (alias: uninstall)\n"
              << "  " << green() << "list" << reset() << "                    List installed packages\n"
              << "  " << green() << "update" << reset() << "                  List available updates\n"
              << "  " << green() << "upgrade" << reset() << " [pkg]           Apply updates, all of them or a single package\n"
              << "  " << green() << "info" << reset() << " <pkg>              Display metadata and details for a package\n"
              << "  " << green() << "sources" << reset() << "                 List registered package plugins & sources (alias: plugins)\n"
              << "  " << green() << "gui" << reset() << "                     Launch the graphical interface (alias: --gui, -g)\n"
              << "  " << green() << "tray" << reset() << "                    Launch minimized in system tray (alias: --tray, -t)\n\n"
              << bold() << "OPTIONS:" << reset() << "\n"
              << "  " << yellow() << "-s, --source" << reset() << " <name>     Target a specific package source (Flatpak, Pacman, AUR, AppImage, ...)\n"
              << "  " << yellow() << "--scope" << reset() << " <user|system>   Installation scope, Flatpak only (default: where the package is)\n"
              << "  " << yellow() << "--json" << reset() << "                  Print machine readable output\n"
              << "  " << yellow() << "--no-color" << reset() << "              Disable coloured output (also honours NO_COLOR)\n"
              << "  " << yellow() << "-h, --help" << reset() << "              Show this help message\n"
              << "  " << yellow() << "-v, --version" << reset() << "           Show version information\n\n"
              << bold() << "EXAMPLES:" << reset() << "\n"
              << "  astra search discord\n"
              << "  astra search vlc --source Flatpak --json\n"
              << "  astra install org.videolan.VLC --scope system\n"
              << "  astra install yay-bin --source AUR\n"
              << "  astra upgrade\n"
              << "  astra info com.spotify.Client\n\n"
              << bold() << "EXIT STATUS:" << reset() << "\n"
              << "  0  success\n"
              << "  1  the requested operation failed or the arguments were invalid\n";
}

CliHandler::Options CliHandler::parseOptions(const QStringList& args) {
    Options options;

    for (int i = 0; i < args.size(); ++i) {
        const QString& argument = args.at(i);

        if ((argument == QStringLiteral("-s") || argument == QStringLiteral("--source")) && i + 1 < args.size()) {
            options.source = args.at(++i);
        } else if (argument == QStringLiteral("--scope") && i + 1 < args.size()) {
            options.scope = args.at(++i);
        } else if (argument == QStringLiteral("--json")) {
            options.json = true;
        } else if (argument == QStringLiteral("--no-color") || argument == QStringLiteral("--no-colour")) {
            continue;
        } else {
            options.positional.append(argument);
        }
    }

    return options;
}

IPackagePlugin* CliHandler::resolveSource(const QString& packageId, const Options& options, PackageManager& pm, bool installedOnly, bool requireUnique) {
    if (!options.source.isEmpty()) {
        IPackagePlugin* plugin = pm.findPlugin(options.source);
        if (!plugin) {
            std::cerr << red() << "Error: source \"" << text(options.source) << "\" not found." << reset() << "\n";
        }
        return plugin;
    }

    QList<IPackagePlugin*> candidates;
    for (IPackagePlugin* plugin : pm.plugins()) {
        if (!plugin->isAvailable() || !plugin->isEnabled()) continue;

        if (installedOnly) {
            if (pm.isPackageInstalled(plugin->id(), packageId)) candidates.append(plugin);
            continue;
        }

        const QVariantList results = plugin->search(packageId);
        for (const QVariant& value : results) {
            if (value.toMap().value(QStringLiteral("id")).toString().compare(packageId, Qt::CaseInsensitive) == 0) {
                candidates.append(plugin);
                break;
            }
        }
    }

    if (candidates.size() == 1) return candidates.first();

    if (candidates.size() > 1) {
        if (!requireUnique) {
            std::cout << dim() << "Using " << text(candidates.first()->name()) << ", also available from ";
            for (int i = 1; i < candidates.size(); ++i) {
                if (i > 1) std::cout << ", ";
                std::cout << text(candidates.at(i)->name());
            }
            std::cout << " (--source picks another)" << reset() << "\n";
            return candidates.first();
        }

        std::cerr << yellow() << "\"" << text(packageId) << "\" is available from several sources:" << reset() << "\n";
        for (IPackagePlugin* plugin : candidates) {
            std::cerr << "  " << backendColour(plugin->id()) << text(plugin->name()) << reset() << "\n";
        }
        std::cerr << "Pick one with " << bold() << "--source <name>" << reset() << ".\n";
        return nullptr;
    }

    if (installedOnly) {
        std::cerr << red() << "Error: \"" << text(packageId) << "\" is not installed." << reset() << "\n";
        return nullptr;
    }

    IPackagePlugin* fallback = pm.findPlugin(packageId.contains(QLatin1Char('.')) ? QStringLiteral("Flatpak") : QStringLiteral("Pacman"));
    if (!fallback) {
        std::cerr << red() << "Error: no source provides \"" << text(packageId) << "\"." << reset() << "\n";
    }
    return fallback;
}

int CliHandler::installAppImageFile(const QString& path) {
    AppImageInstaller installer;
    std::cout << bold() << "Installing AppImage: " << cyan() << text(path) << reset() << "...\n";

    if (installer.installAppImage(path)) {
        std::cout << green() << bold() << "Successfully installed AppImage" << reset() << "\n";
        std::cout << dim() << "  Desktop entry registered in ~/.local/share/applications" << reset() << "\n";
        return 0;
    }

    std::cerr << red() << bold() << "Failed to install AppImage: " << text(installer.statusMessage()) << reset() << "\n";
    return 1;
}

int CliHandler::run(int argc, char* argv[], PackageManager& pm) {
    QStringList args;
    for (int i = 1; i < argc; ++i) {
        args.append(QString::fromUtf8(argv[i]));
    }

    g_colours = isatty(STDOUT_FILENO) && !qEnvironmentVariableIsSet("NO_COLOR") &&
                !args.contains(QStringLiteral("--no-color")) && !args.contains(QStringLiteral("--no-colour"));

    if (args.isEmpty()) {
        printHelp();
        return 0;
    }

    const QString command = args.takeFirst().toLower();
    const Options options = parseOptions(args);

    if (command == QStringLiteral("--help") || command == QStringLiteral("-h") || command == QStringLiteral("help")) {
        printHelp();
        return 0;
    }
    if (command == QStringLiteral("--version") || command == QStringLiteral("-v") || command == QStringLiteral("version")) {
        printVersion();
        return 0;
    }
    if (command == QStringLiteral("search")) return handleSearch(options, pm);
    if (command == QStringLiteral("install")) return handleInstall(options, pm);
    if (command == QStringLiteral("remove") || command == QStringLiteral("uninstall")) return handleUninstall(options, pm);
    if (command == QStringLiteral("list") || command == QStringLiteral("installed")) return handleList(options, pm);
    if (command == QStringLiteral("update")) return handleUpdate(options, pm);
    if (command == QStringLiteral("upgrade")) return handleUpgrade(options, pm);
    if (command == QStringLiteral("info")) return handleInfo(options, pm);
    if (command == QStringLiteral("sources") || command == QStringLiteral("plugins")) return handleSources(options, pm);

    if (looksLikeAppImage(command) || QFile::exists(command)) {
        return installAppImageFile(command);
    }

    std::cerr << red() << "Unknown command: " << text(command) << reset() << "\n";
    std::cout << "Run " << bold() << "astra --help" << reset() << " for available commands.\n";
    return 1;
}

int CliHandler::handleSources(const Options& options, PackageManager& pm) {
    const QVariantList plugins = pm.getRegisteredPluginsInfo();

    if (options.json) {
        printJson(QJsonDocument(toJsonArray(plugins)));
        return 0;
    }

    std::cout << bold() << cyan() << "Registered Package Sources & Plugins:" << reset() << "\n\n";
    for (const QVariant& value : plugins) {
        const QVariantMap plugin = value.toMap();
        const bool available = plugin.value(QStringLiteral("isAvailable")).toBool();
        const bool enabled = plugin.value(QStringLiteral("isEnabled")).toBool();

        std::cout << "  - " << bold() << text(plugin.value(QStringLiteral("name")).toString()) << reset();
        if (available && enabled) {
            std::cout << " [" << green() << "active" << reset() << "]";
        } else if (!available) {
            std::cout << " [" << red() << "unavailable" << reset() << "]";
        } else {
            std::cout << " [" << yellow() << "disabled" << reset() << "]";
        }
        std::cout << "\n    " << dim() << text(plugin.value(QStringLiteral("description")).toString()) << reset() << "\n\n";
    }
    return 0;
}

int CliHandler::handleSearch(const Options& options, PackageManager& pm) {
    const QString query = options.positional.value(0);
    if (query.isEmpty()) {
        std::cerr << red() << "Error: search query required." << reset() << "\n";
        std::cout << "Usage: astra search <query> [--source <source>] [--json]\n";
        return 1;
    }

    const QVariantList results = pm.searchPackages(query, options.source);

    if (options.json) {
        printJson(QJsonDocument(toJsonArray(results)));
        return 0;
    }

    std::cout << dim() << "Searching for \"" << text(query) << "\"..." << reset() << "\n\n";
    if (results.isEmpty()) {
        std::cout << yellow() << "No packages found matching \"" << text(query) << "\"." << reset() << "\n";
        return 0;
    }

    std::cout << bold() << std::left << std::setw(36) << "PACKAGE ID" << std::setw(12) << "SOURCE" << std::setw(24) << "NAME"
              << "SUMMARY" << reset() << "\n";
    std::cout << std::string(90, '-') << "\n";

    for (const QVariant& value : results) {
        const QVariantMap item = value.toMap();
        const QString backend = item.value(QStringLiteral("backend")).toString();

        std::cout << std::left << std::setw(36) << text(truncated(item.value(QStringLiteral("id")).toString(), 34))
                  << backendColour(backend) << std::setw(12) << text(backend) << reset()
                  << std::setw(24) << text(truncated(item.value(QStringLiteral("name")).toString(), 22))
                  << dim() << text(truncated(item.value(QStringLiteral("summary")).toString(), 40)) << reset() << "\n";
    }

    std::cout << "\nFound " << green() << results.size() << reset() << " packages.\n";
    return 0;
}

int CliHandler::handleInstall(const Options& options, PackageManager& pm) {
    const QString packageId = options.positional.value(0);
    if (packageId.isEmpty()) {
        std::cerr << red() << "Error: package id required." << reset() << "\n";
        std::cout << "Usage: astra install <package-id> [--source <source>] [--scope user|system]\n";
        return 1;
    }

    if (looksLikeAppImage(packageId) || (QFile::exists(packageId) && options.source.compare(QStringLiteral("AppImage"), Qt::CaseInsensitive) == 0)) {
        return installAppImageFile(packageId);
    }

    IPackagePlugin* plugin = resolveSource(packageId, options, pm, false, true);
    if (!plugin) return 1;

    std::cout << bold() << "Installing " << cyan() << text(packageId) << reset() << " from " << green() << text(plugin->name()) << reset();
    if (!options.scope.isEmpty()) std::cout << " (scope: " << text(options.scope) << ")";
    std::cout << "...\n";

    QVariantMap pluginOptions;
    if (!options.scope.isEmpty()) pluginOptions[QStringLiteral("scope")] = options.scope;

    if (plugin->install(packageId, pluginOptions, reportProgress)) {
        std::cout << green() << bold() << "Successfully installed " << text(packageId) << reset() << "\n";
        return 0;
    }

    std::cerr << red() << bold() << "Failed to install " << text(packageId) << reset() << "\n";
    return 1;
}

int CliHandler::handleUninstall(const Options& options, PackageManager& pm) {
    const QString packageId = options.positional.value(0);
    if (packageId.isEmpty()) {
        std::cerr << red() << "Error: package id required." << reset() << "\n";
        std::cout << "Usage: astra remove <package-id> [--source <source>] [--scope user|system]\n";
        return 1;
    }

    IPackagePlugin* plugin = resolveSource(packageId, options, pm, true, true);
    if (!plugin) return 1;

    std::cout << bold() << "Uninstalling " << cyan() << text(packageId) << reset() << " (" << text(plugin->name()) << ")...\n";

    QVariantMap pluginOptions;
    if (!options.scope.isEmpty()) pluginOptions[QStringLiteral("scope")] = options.scope;

    if (plugin->uninstall(packageId, pluginOptions, reportProgress)) {
        std::cout << green() << bold() << "Successfully removed " << text(packageId) << reset() << "\n";
        return 0;
    }

    std::cerr << red() << bold() << "Failed to remove " << text(packageId) << reset() << "\n";
    return 1;
}

int CliHandler::handleList(const Options& options, PackageManager& pm) {
    QVariantList installed = pm.getInstalledPackages();

    if (!options.source.isEmpty()) {
        QVariantList filtered;
        for (const QVariant& value : installed) {
            if (value.toMap().value(QStringLiteral("backend")).toString().compare(options.source, Qt::CaseInsensitive) == 0) {
                filtered.append(value);
            }
        }
        installed = filtered;
    }

    if (options.json) {
        printJson(QJsonDocument(toJsonArray(installed)));
        return 0;
    }

    std::cout << dim() << "Listing installed packages..." << reset() << "\n\n";
    if (installed.isEmpty()) {
        std::cout << "No installed packages found.\n";
        return 0;
    }

    std::cout << bold() << std::left << std::setw(36) << "PACKAGE ID" << std::setw(12) << "SOURCE" << std::setw(18) << "VERSION"
              << "SCOPE" << reset() << "\n";
    std::cout << std::string(75, '-') << "\n";

    for (const QVariant& value : installed) {
        const QVariantMap item = value.toMap();
        std::cout << std::left << std::setw(36) << text(truncated(item.value(QStringLiteral("id")).toString(), 34))
                  << std::setw(12) << text(item.value(QStringLiteral("backend")).toString())
                  << std::setw(18) << text(truncated(item.value(QStringLiteral("version")).toString(), 16))
                  << dim() << text(item.value(QStringLiteral("scope")).toString()) << reset() << "\n";
    }

    std::cout << "\nTotal installed: " << green() << installed.size() << reset() << "\n";
    return 0;
}

int CliHandler::handleUpdate(const Options& options, PackageManager& pm) {
    const QVariantList updates = pm.checkForUpdates();

    if (options.json) {
        printJson(QJsonDocument(toJsonArray(updates)));
        return 0;
    }

    std::cout << dim() << "Checking for updates across all sources..." << reset() << "\n\n";
    if (updates.isEmpty()) {
        std::cout << green() << "All packages are up to date." << reset() << "\n";
        return 0;
    }

    std::cout << bold() << "Available Updates (" << updates.size() << "):" << reset() << "\n";
    for (const QVariant& value : updates) {
        const QVariantMap item = value.toMap();
        std::cout << "  - " << text(item.value(QStringLiteral("name")).toString()) << " ["
                  << text(item.value(QStringLiteral("backend")).toString()) << "] " << yellow()
                  << text(item.value(QStringLiteral("version")).toString()) << reset() << "\n";
    }
    std::cout << "\nApply them with " << bold() << "astra upgrade" << reset() << ".\n";
    return 0;
}

int CliHandler::handleUpgrade(const Options& options, PackageManager& pm) {
    const QString packageId = options.positional.value(0);

    if (packageId.isEmpty()) {
        std::cout << bold() << "Applying updates..." << reset() << "\n";
        const bool success = pm.runSystemUpdate([](const QString& line) {
            std::cout << dim() << "  " << text(line) << reset() << "\n";
        });

        if (success) {
            std::cout << green() << bold() << "Updates applied" << reset() << "\n";
            return 0;
        }
        std::cerr << red() << bold() << "Some updates could not be applied" << reset() << "\n";
        return 1;
    }

    IPackagePlugin* plugin = resolveSource(packageId, options, pm, true, true);
    if (!plugin) return 1;

    std::cout << bold() << "Updating " << cyan() << text(packageId) << reset() << " (" << text(plugin->name()) << ")...\n";

    QVariantMap pluginOptions;
    if (!options.scope.isEmpty()) pluginOptions[QStringLiteral("scope")] = options.scope;

    if (plugin->update(packageId, pluginOptions, reportProgress)) {
        std::cout << green() << bold() << "Successfully updated " << text(packageId) << reset() << "\n";
        return 0;
    }

    std::cerr << red() << bold() << "Failed to update " << text(packageId) << reset() << "\n";
    return 1;
}

int CliHandler::handleInfo(const Options& options, PackageManager& pm) {
    const QString packageId = options.positional.value(0);
    if (packageId.isEmpty()) {
        std::cerr << red() << "Error: package id required." << reset() << "\n";
        std::cout << "Usage: astra info <package-id> [--source <source>] [--json]\n";
        return 1;
    }

    IPackagePlugin* plugin = resolveSource(packageId, options, pm, false, false);
    if (!plugin) return 1;

    const QVariantMap details = plugin->getDetails(packageId);

    if (options.json) {
        printJson(QJsonDocument(QJsonObject::fromVariantMap(details)));
        return 0;
    }

    const QString name = details.value(QStringLiteral("name")).toString();
    std::cout << bold() << cyan() << text(name.isEmpty() ? packageId : name) << reset() << "\n";
    std::cout << std::string(50, '=') << "\n";

    const auto field = [&details](const char* label, const QString& key) {
        const QString value = details.value(key).toString();
        if (value.isEmpty()) return;
        std::cout << bold() << label << reset() << text(value) << "\n";
    };

    field("ID:          ", QStringLiteral("id"));
    field("Backend:     ", QStringLiteral("backend"));
    field("Version:     ", QStringLiteral("version"));
    field("Repository:  ", QStringLiteral("repository"));
    field("Developer:   ", QStringLiteral("developer"));
    field("License:     ", QStringLiteral("license"));
    field("Size:        ", QStringLiteral("installedSize"));
    field("Homepage:    ", QStringLiteral("homepage"));
    field("Summary:     ", QStringLiteral("summary"));

    const QString description = details.value(QStringLiteral("description")).toString();
    if (!description.isEmpty()) {
        std::cout << "\n" << bold() << "Description:" << reset() << "\n" << text(description) << "\n";
    }

    const QList<QVariantMap> sources = pm.getInstallSources(plugin->id(), packageId);
    if (!sources.isEmpty()) {
        std::cout << "\n" << bold() << "Available Installation Scopes:" << reset() << "\n";
        for (const QVariantMap& source : sources) {
            std::cout << "  - " << text(source.value(QStringLiteral("label")).toString()) << " (--scope "
                      << text(source.value(QStringLiteral("id")).toString()) << ")\n";
        }
    }
    return 0;
}
