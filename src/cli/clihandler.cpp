#include "clihandler.hpp"
#include <iostream>
#include <iomanip>
#include <QCoreApplication>
#include <QFile>
#include "marketplace/appimageinstaller.hpp"

#define ANSI_RESET   "\033[0m"
#define ANSI_BOLD    "\033[1m"
#define ANSI_DIM     "\033[2m"
#define ANSI_CYAN    "\033[36m"
#define ANSI_GREEN   "\033[32m"
#define ANSI_YELLOW  "\033[33m"
#define ANSI_RED     "\033[31m"
#define ANSI_MAGENTA "\033[35m"

void CliHandler::printVersion() {
    std::cout << ANSI_BOLD ANSI_CYAN "astra" ANSI_RESET " version " ANSI_GREEN "1.0.4" ANSI_RESET " (AstraMarket Universal Package Manager)\n";
}

void CliHandler::printHelp() {
    std::cout << ANSI_BOLD ANSI_CYAN "AstraMarket CLI" ANSI_RESET " - Unified Linux package management\n\n"
              << ANSI_BOLD "USAGE:" ANSI_RESET "\n"
              << "  astra <COMMAND> [OPTIONS]\n\n"
              << ANSI_BOLD "COMMANDS:" ANSI_RESET "\n"
              << "  " ANSI_GREEN "search" ANSI_RESET " <query>          Search for packages across all or specific sources\n"
              << "  " ANSI_GREEN "install" ANSI_RESET " <pkg>           Install a package\n"
              << "  " ANSI_GREEN "remove" ANSI_RESET " <pkg>            Uninstall a package (alias: uninstall)\n"
              << "  " ANSI_GREEN "list" ANSI_RESET "                  List installed packages\n"
              << "  " ANSI_GREEN "update" ANSI_RESET "                Check for or perform updates (alias: upgrade)\n"
              << "  " ANSI_GREEN "info" ANSI_RESET " <pkg>              Display metadata and details for a package\n"
              << "  " ANSI_GREEN "sources" ANSI_RESET "               List registered package plugins & sources (alias: plugins)\n"
              << "  " ANSI_GREEN "gui" ANSI_RESET "                   Launch the graphical interface (alias: --gui, -g)\n\n"
              << ANSI_BOLD "OPTIONS:" ANSI_RESET "\n"
              << "  " ANSI_YELLOW "-s, --source" ANSI_RESET " <name>     Target specific package source (Flatpak, Pacman, AUR, AppImage, etc.)\n"
              << "  " ANSI_YELLOW "--scope" ANSI_RESET " <user|system>    Specify installation scope (user or system, default: user for Flatpak)\n"
              << "  " ANSI_YELLOW "-h, --help" ANSI_RESET "              Show this help message\n"
              << "  " ANSI_YELLOW "-v, --version" ANSI_RESET "           Show version information\n\n"
              << ANSI_BOLD "EXAMPLES:" ANSI_RESET "\n"
              << "  astra search discord\n"
              << "  astra search vlc --source Flatpak\n"
              << "  astra install org.videolan.VLC --scope system\n"
              << "  astra info com.spotify.Client\n"
              << "  astra sources\n";
}

int CliHandler::run(int argc, char* argv[], PackageManager& pm) {
    if (argc < 2) {
        printHelp();
        return 0;
    }

    QStringList args;
    for (int i = 1; i < argc; ++i) {
        args.append(QString::fromUtf8(argv[i]));
    }

    QString cmd = args.first().toLower();
    args.removeFirst();

    if (cmd == QStringLiteral("--help") || cmd == QStringLiteral("-h") || cmd == QStringLiteral("help")) {
        printHelp();
        return 0;
    }
    if (cmd == QStringLiteral("--version") || cmd == QStringLiteral("-v") || cmd == QStringLiteral("version")) {
        printVersion();
        return 0;
    }
    if (cmd == QStringLiteral("search")) {
        return handleSearch(args, pm);
    }
    if (cmd == QStringLiteral("install")) {
        return handleInstall(args, pm);
    }
    if (cmd == QStringLiteral("remove") || cmd == QStringLiteral("uninstall")) {
        return handleUninstall(args, pm);
    }
    if (cmd == QStringLiteral("list") || cmd == QStringLiteral("installed")) {
        return handleList(args, pm);
    }
    if (cmd == QStringLiteral("update") || cmd == QStringLiteral("upgrade")) {
        return handleUpdate(args, pm);
    }
    if (cmd == QStringLiteral("info")) {
        return handleInfo(args, pm);
    }
    if (cmd == QStringLiteral("sources") || cmd == QStringLiteral("plugins")) {
        return handleSources(args, pm);
    }

    if (cmd.endsWith(QLatin1String(".appimage"), Qt::CaseInsensitive) ||
        cmd.startsWith(QLatin1String("file://")) ||
        QFile::exists(cmd)) {
        AppImageInstaller installer;
        std::cout << ANSI_BOLD "Installing AppImage: " ANSI_CYAN << cmd.toStdString() << ANSI_RESET "...\n";
        bool ok = installer.installAppImage(cmd);
        if (ok) {
            std::cout << ANSI_GREEN ANSI_BOLD "✓ Successfully installed AppImage!" ANSI_RESET "\n";
            std::cout << ANSI_DIM "  Desktop entry registered in ~/.local/share/applications" ANSI_RESET "\n";
            return 0;
        } else {
            std::cerr << ANSI_RED ANSI_BOLD "✗ Failed to install AppImage: " << installer.statusMessage().toStdString() << ANSI_RESET "\n";
            return 1;
        }
    }

    std::cerr << ANSI_RED "Unknown command: " << cmd.toStdString() << ANSI_RESET "\n";
    std::cout << "Run " ANSI_BOLD "astra --help" ANSI_RESET " for available commands.\n";
    return 1;
}

int CliHandler::handleSources(const QStringList& args, PackageManager& pm) {
    Q_UNUSED(args);
    std::cout << ANSI_BOLD ANSI_CYAN "Registered Package Sources & Plugins:" ANSI_RESET "\n\n";

    QVariantList plugins = pm.getRegisteredPluginsInfo();
    for (const QVariant& p : plugins) {
        QVariantMap map = p.toMap();
        QString name = map.value(QStringLiteral("name")).toString();
        QString desc = map.value(QStringLiteral("description")).toString();
        bool isAvail = map.value(QStringLiteral("isAvailable")).toBool();
        bool isEnabled = map.value(QStringLiteral("isEnabled")).toBool();

        std::cout << "  • " << ANSI_BOLD << name.toStdString() << ANSI_RESET;
        if (isAvail && isEnabled) {
            std::cout << " [" ANSI_GREEN "active" ANSI_RESET "]";
        } else if (!isAvail) {
            std::cout << " [" ANSI_RED "unavailable" ANSI_RESET "]";
        } else {
            std::cout << " [" ANSI_YELLOW "disabled" ANSI_RESET "]";
        }
        std::cout << "\n    " << ANSI_DIM << desc.toStdString() << ANSI_RESET "\n\n";
    }
    return 0;
}

int CliHandler::handleSearch(const QStringList& args, PackageManager& pm) {
    QString query;
    QString source;

    for (int i = 0; i < args.size(); ++i) {
        if ((args[i] == QStringLiteral("-s") || args[i] == QStringLiteral("--source")) && i + 1 < args.size()) {
            source = args[++i];
        } else if (query.isEmpty()) {
            query = args[i];
        }
    }

    if (query.isEmpty()) {
        std::cerr << ANSI_RED "Error: Search query required." ANSI_RESET "\n";
        std::cout << "Usage: astra search <query> [--source <source>]\n";
        return 1;
    }

    std::cout << ANSI_DIM "Searching for \"" << query.toStdString() << "\"..." ANSI_RESET "\n\n";

    QVariantList results = pm.searchPackages(query, source);
    if (results.isEmpty()) {
        std::cout << ANSI_YELLOW "No packages found matching \"" << query.toStdString() << "\"." ANSI_RESET "\n";
        return 0;
    }

    std::cout << ANSI_BOLD
              << std::left << std::setw(36) << "PACKAGE ID"
              << std::setw(12) << "SOURCE"
              << std::setw(24) << "NAME"
              << "SUMMARY"
              << ANSI_RESET "\n";

    std::cout << std::string(90, '-') << "\n";

    for (const QVariant& v : results) {
        QVariantMap map = v.toMap();
        QString id = map.value(QStringLiteral("id")).toString();
        QString backend = map.value(QStringLiteral("backend")).toString();
        QString name = map.value(QStringLiteral("name")).toString();
        QString summary = map.value(QStringLiteral("summary")).toString();

        if (id.length() > 34) id = id.left(31) + QStringLiteral("...");
        if (name.length() > 22) name = name.left(19) + QStringLiteral("...");
        if (summary.length() > 40) summary = summary.left(37) + QStringLiteral("...");

        std::string backendColor = ANSI_CYAN;
        if (backend == QStringLiteral("Pacman")) backendColor = ANSI_GREEN;
        else if (backend == QStringLiteral("AUR")) backendColor = ANSI_MAGENTA;
        else if (backend == QStringLiteral("AppImage")) backendColor = ANSI_YELLOW;

        std::cout << std::left << std::setw(36) << id.toStdString()
                  << backendColor << std::setw(12) << backend.toStdString() << ANSI_RESET
                  << std::setw(24) << name.toStdString()
                  << ANSI_DIM << summary.toStdString() << ANSI_RESET
                  << "\n";
    }

    std::cout << "\nFound " ANSI_GREEN << results.size() << ANSI_RESET " packages.\n";
    return 0;
}

int CliHandler::handleInstall(const QStringList& args, PackageManager& pm) {
    QString packageId;
    QString source;
    QString scope = QStringLiteral("user");

    for (int i = 0; i < args.size(); ++i) {
        if ((args[i] == QStringLiteral("-s") || args[i] == QStringLiteral("--source")) && i + 1 < args.size()) {
            source = args[++i];
        } else if (args[i] == QStringLiteral("--scope") && i + 1 < args.size()) {
            scope = args[++i];
        } else if (packageId.isEmpty()) {
            packageId = args[i];
        }
    }

    if (packageId.isEmpty()) {
        std::cerr << ANSI_RED "Error: Package ID required." ANSI_RESET "\n";
        std::cout << "Usage: astra install <package-id> [--source <source>] [--scope user|system]\n";
        return 1;
    }

    if (packageId.endsWith(QLatin1String(".AppImage"), Qt::CaseInsensitive) ||
        packageId.endsWith(QLatin1String(".appimage"), Qt::CaseInsensitive) ||
        packageId.startsWith(QLatin1String("file://")) ||
        (QFile::exists(packageId) && source == QStringLiteral("AppImage"))) {
        AppImageInstaller installer;
        std::cout << ANSI_BOLD "Installing AppImage: " ANSI_CYAN << packageId.toStdString() << ANSI_RESET "...\n";
        bool ok = installer.installAppImage(packageId);
        if (ok) {
            std::cout << ANSI_GREEN ANSI_BOLD "✓ Successfully installed AppImage: " << packageId.toStdString() << ANSI_RESET "\n";
            std::cout << ANSI_DIM "  Desktop entry registered in ~/.local/share/applications" ANSI_RESET "\n";
            return 0;
        } else {
            std::cerr << ANSI_RED ANSI_BOLD "✗ Failed to install AppImage: " << installer.statusMessage().toStdString() << ANSI_RESET "\n";
            return 1;
        }
    }

    if (source.isEmpty()) {
        if (packageId.contains(QLatin1Char('.'))) {
            source = QStringLiteral("Flatpak");
        } else {
            source = QStringLiteral("Pacman");
        }
    }

    IPackagePlugin* plugin = pm.findPlugin(source);
    if (!plugin) {
        std::cerr << ANSI_RED "Error: Source plugin \"" << source.toStdString() << "\" not found." ANSI_RESET "\n";
        return 1;
    }

    std::cout << ANSI_BOLD "Installing " ANSI_CYAN << packageId.toStdString() << ANSI_RESET
              << " from " ANSI_GREEN << plugin->name().toStdString() << ANSI_RESET
              << " (scope: " << scope.toStdString() << ")...\n";

    QVariantMap options;
    options[QStringLiteral("scope")] = scope;

    bool ok = plugin->install(packageId, options, [](int pct, const QString& status) {
        Q_UNUSED(pct);
        if (!status.isEmpty()) {
            std::cout << ANSI_DIM << "  " << status.toStdString() << ANSI_RESET << "\n";
        }
    });

    if (ok) {
        std::cout << ANSI_GREEN ANSI_BOLD "✓ Successfully installed " << packageId.toStdString() << ANSI_RESET "\n";
        return 0;
    } else {
        std::cerr << ANSI_RED ANSI_BOLD "✗ Failed to install " << packageId.toStdString() << ANSI_RESET "\n";
        return 1;
    }
}

int CliHandler::handleUninstall(const QStringList& args, PackageManager& pm) {
    QString packageId;
    QString source;

    for (int i = 0; i < args.size(); ++i) {
        if ((args[i] == QStringLiteral("-s") || args[i] == QStringLiteral("--source")) && i + 1 < args.size()) {
            source = args[++i];
        } else if (packageId.isEmpty()) {
            packageId = args[i];
        }
    }

    if (packageId.isEmpty()) {
        std::cerr << ANSI_RED "Error: Package ID required." ANSI_RESET "\n";
        std::cout << "Usage: astra remove <package-id> [--source <source>]\n";
        return 1;
    }

    if (source.isEmpty()) {
        if (packageId.contains(QLatin1Char('.'))) {
            source = QStringLiteral("Flatpak");
        } else {
            source = QStringLiteral("Pacman");
        }
    }

    IPackagePlugin* plugin = pm.findPlugin(source);
    if (!plugin) {
        std::cerr << ANSI_RED "Error: Source plugin \"" << source.toStdString() << "\" not found." ANSI_RESET "\n";
        return 1;
    }

    std::cout << ANSI_BOLD "Uninstalling " ANSI_CYAN << packageId.toStdString() << ANSI_RESET "...\n";

    bool ok = plugin->uninstall(packageId, {}, [](int pct, const QString& status) {
        Q_UNUSED(pct);
        if (!status.isEmpty()) {
            std::cout << ANSI_DIM << "  " << status.toStdString() << ANSI_RESET << "\n";
        }
    });

    if (ok) {
        std::cout << ANSI_GREEN ANSI_BOLD "✓ Successfully removed " << packageId.toStdString() << ANSI_RESET "\n";
        return 0;
    } else {
        std::cerr << ANSI_RED ANSI_BOLD "✗ Failed to remove " << packageId.toStdString() << ANSI_RESET "\n";
        return 1;
    }
}

int CliHandler::handleList(const QStringList& args, PackageManager& pm) {
    Q_UNUSED(args);
    std::cout << ANSI_DIM "Listing installed packages..." ANSI_RESET "\n\n";

    QVariantList installed = pm.getInstalledPackages();
    if (installed.isEmpty()) {
        std::cout << "No installed packages found.\n";
        return 0;
    }

    std::cout << ANSI_BOLD
              << std::left << std::setw(36) << "PACKAGE ID"
              << std::setw(12) << "SOURCE"
              << std::setw(18) << "VERSION"
              << "SCOPE"
              << ANSI_RESET "\n";
    std::cout << std::string(75, '-') << "\n";

    for (const QVariant& v : installed) {
        QVariantMap map = v.toMap();
        QString id = map.value(QStringLiteral("id")).toString();
        QString backend = map.value(QStringLiteral("backend")).toString();
        QString version = map.value(QStringLiteral("version")).toString();
        QString scope = map.value(QStringLiteral("scope")).toString();

        if (id.length() > 34) id = id.left(31) + QStringLiteral("...");
        if (version.length() > 16) version = version.left(13) + QStringLiteral("...");

        std::cout << std::left << std::setw(36) << id.toStdString()
                  << std::setw(12) << backend.toStdString()
                  << std::setw(18) << version.toStdString()
                  << ANSI_DIM << scope.toStdString() << ANSI_RESET
                  << "\n";
    }

    std::cout << "\nTotal installed: " ANSI_GREEN << installed.size() << ANSI_RESET "\n";
    return 0;
}

int CliHandler::handleUpdate(const QStringList& args, PackageManager& pm) {
    Q_UNUSED(args);
    std::cout << ANSI_DIM "Checking for updates across all sources..." ANSI_RESET "\n\n";

    QVariantList updates = pm.checkForUpdates();
    if (updates.isEmpty()) {
        std::cout << ANSI_GREEN "✓ All packages are up to date." ANSI_RESET "\n";
        return 0;
    }

    std::cout << ANSI_BOLD "Available Updates (" << updates.size() << "):" ANSI_RESET "\n";
    for (const QVariant& v : updates) {
        QVariantMap map = v.toMap();
        std::cout << "  • " << map.value(QStringLiteral("name")).toString().toStdString()
                  << " [" << map.value(QStringLiteral("backend")).toString().toStdString() << "] "
                  << ANSI_YELLOW << map.value(QStringLiteral("version")).toString().toStdString() << ANSI_RESET "\n";
    }

    return 0;
}

int CliHandler::handleInfo(const QStringList& args, PackageManager& pm) {
    QString packageId;
    QString source;

    for (int i = 0; i < args.size(); ++i) {
        if ((args[i] == QStringLiteral("-s") || args[i] == QStringLiteral("--source")) && i + 1 < args.size()) {
            source = args[++i];
        } else if (packageId.isEmpty()) {
            packageId = args[i];
        }
    }

    if (packageId.isEmpty()) {
        std::cerr << ANSI_RED "Error: Package ID required." ANSI_RESET "\n";
        std::cout << "Usage: astra info <package-id> [--source <source>]\n";
        return 1;
    }

    if (source.isEmpty()) {
        if (packageId.contains(QLatin1Char('.'))) {
            source = QStringLiteral("Flatpak");
        } else {
            source = QStringLiteral("Pacman");
        }
    }

    QVariantMap details = pm.getPackageDetails(packageId, source);
    std::cout << ANSI_BOLD ANSI_CYAN << (details.value(QStringLiteral("name")).toString().isEmpty() ? packageId.toStdString() : details.value(QStringLiteral("name")).toString().toStdString()) << ANSI_RESET "\n";
    std::cout << std::string(50, '=') << "\n";
    std::cout << ANSI_BOLD "ID:          " ANSI_RESET << details.value(QStringLiteral("id")).toString().toStdString() << "\n";
    std::cout << ANSI_BOLD "Backend:     " ANSI_RESET << details.value(QStringLiteral("backend")).toString().toStdString() << "\n";
    if (!details.value(QStringLiteral("version")).toString().isEmpty())
        std::cout << ANSI_BOLD "Version:     " ANSI_RESET << details.value(QStringLiteral("version")).toString().toStdString() << "\n";
    if (!details.value(QStringLiteral("developer")).toString().isEmpty())
        std::cout << ANSI_BOLD "Developer:   " ANSI_RESET << details.value(QStringLiteral("developer")).toString().toStdString() << "\n";
    if (!details.value(QStringLiteral("license")).toString().isEmpty())
        std::cout << ANSI_BOLD "License:     " ANSI_RESET << details.value(QStringLiteral("license")).toString().toStdString() << "\n";
    if (!details.value(QStringLiteral("homepage")).toString().isEmpty())
        std::cout << ANSI_BOLD "Homepage:    " ANSI_RESET << details.value(QStringLiteral("homepage")).toString().toStdString() << "\n";
    if (!details.value(QStringLiteral("summary")).toString().isEmpty())
        std::cout << ANSI_BOLD "Summary:     " ANSI_RESET << details.value(QStringLiteral("summary")).toString().toStdString() << "\n";
    if (!details.value(QStringLiteral("description")).toString().isEmpty()) {
        std::cout << "\n" ANSI_BOLD "Description:" ANSI_RESET "\n" << details.value(QStringLiteral("description")).toString().toStdString() << "\n";
    }

    QList<QVariantMap> sources = pm.getInstallSources(source, packageId);
    if (!sources.isEmpty()) {
        std::cout << "\n" ANSI_BOLD "Available Installation Scopes:" ANSI_RESET "\n";
        for (const auto& s : sources) {
            std::cout << "  • " << s.value(QStringLiteral("label")).toString().toStdString()
                      << " (--scope " << s.value(QStringLiteral("id")).toString().toStdString() << ")\n";
        }
    }
    return 0;
}
