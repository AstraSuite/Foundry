#include <QGuiApplication>
#include <QSurfaceFormat>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFontDatabase>
#include <QIcon>
#include <QDir>
#include <QCommandLineParser>
#include <QLocale>
#include <QTranslator>
#include <QStandardPaths>
#include <QCommandLineOption>
#include <QProcess>
#include <iostream>

#include "themewatcher.hpp"
#include "utils/cutils.hpp"
#include "config/tokensattached.hpp"
#include "config/tokens.hpp"
#include "config/appearanceconfig.hpp"
#include "config/font.hpp"
#include "config/anim.hpp"
#include "blobs/blobgroup.hpp"
#include "blobs/blobinvertedrect.hpp"
#include "blobs/blobrect.hpp"
#include "blobs/blobmaterial.hpp"
#include "marketplace/appimageinstaller.hpp"
#include "marketplace/packagemanager.hpp"
#include "config/tokens.hpp"
#include "config/config.hpp"

#include "utils/iconimageprovider.hpp"

#include "cli/clihandler.hpp"
#include "tray/traymanager.hpp"
#include <QApplication>
#include <QQmlContext>
#include <QLocalServer>
#include <QLocalSocket>

#ifndef ASTRA_VERSION
#define ASTRA_VERSION "1.2.0"
#endif

static void migrateLegacyPaths() {
    const QString configHome = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    const QString dataHome = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);

    const QList<QPair<QString, QString>> locations{
        {configHome + QStringLiteral("/AstraMarket"), configHome + QStringLiteral("/astra-foundry")},
        {dataHome + QStringLiteral("/AstraMarket"), dataHome + QStringLiteral("/astra-foundry")}
    };

    for (const auto& [legacy, current] : locations) {
        if (!QFileInfo::exists(legacy) || QFileInfo::exists(current)) continue;
        QDir().rename(legacy, current);
    }
}

static void installTranslations(QCoreApplication& app) {
    auto* translator = new QTranslator(&app);
    if (translator->load(QLocale(), QStringLiteral("foundry"), QStringLiteral("_"), QStringLiteral(":/i18n"))) {
        QCoreApplication::installTranslator(translator);
    } else {
        delete translator;
    }
}

int main(int argc, char* argv[]) {
    qputenv("QML_XHR_ALLOW_FILE_READ", "1");
    migrateLegacyPaths();
    bool launchGui = false;
    bool launchTray = false;

    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromUtf8(argv[i]);
        if (arg == QStringLiteral("--gui") || arg == QStringLiteral("-g") || arg == QStringLiteral("gui")) {
            launchGui = true;
            continue;
        }
        if (arg == QStringLiteral("--tray") || arg == QStringLiteral("-t") || arg == QStringLiteral("tray")) {
            launchGui = true;
            launchTray = true;
            continue;
        }
        if (arg.endsWith(QLatin1String(".AppImage"), Qt::CaseInsensitive) ||
            arg.endsWith(QLatin1String(".appimage"), Qt::CaseInsensitive) ||
            (arg.startsWith(QLatin1String("file://")) && arg.contains(QLatin1String("appimage"), Qt::CaseInsensitive))) {
            AppImageInstaller installer;
            bool ok = installer.installAppImage(arg);
            if (ok) {
                std::cout << "Successfully installed AppImage: " << arg.toStdString() << "\n";
            }
            return ok ? 0 : 1;
        }
    }

    const QString userName = qgetenv("USER").isEmpty() ? QStringLiteral("default") : QString::fromLocal8Bit(qgetenv("USER"));
    const QString serverName = QStringLiteral("foundry-single-instance-") + userName;

    if (launchGui) {
        QLocalSocket socket;
        socket.connectToServer(serverName);
        if (socket.waitForConnected(500)) {
            // Already running! Signal primary instance to unhide window
            if (launchTray) {
                socket.write("TRAY\n");
            } else {
                socket.write("SHOW\n");
            }
            socket.flush();
            socket.waitForBytesWritten(1000);
            return 0;
        }
    }

    if (!launchGui) {
        QCoreApplication app(argc, argv);
        app.setApplicationName(QStringLiteral("foundry"));
        app.setOrganizationName(QStringLiteral("astra-foundry"));
        app.setApplicationVersion(QStringLiteral(ASTRA_VERSION));

        PackageManager pm;
        return CliHandler::run(argc, argv, pm);
    }

    QSurfaceFormat format;
    format.setRedBufferSize(8);
    format.setGreenBufferSize(8);
    format.setBlueBufferSize(8);
    format.setAlphaBufferSize(8);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("foundry"));
    app.setOrganizationName(QStringLiteral("astra-foundry"));
    app.setDesktopFileName(QStringLiteral("foundry"));
    app.setApplicationVersion(QStringLiteral(ASTRA_VERSION));
    app.setQuitOnLastWindowClosed(false);
    installTranslations(app);

    const QString iconPath = QStringLiteral(":/assets/icons/astra-foundry.svg");
    if (QFile::exists(iconPath)) {
        app.setWindowIcon(QIcon(iconPath));
    } else {
        app.setWindowIcon(QIcon(QStringLiteral("assets/icons/astra-foundry.svg")));
    }

    const QString fontPath = QStringLiteral(":/assets/fonts/GoogleSansFlex-VariableFont_GRAD,ROND,opsz,slnt,wdth,wght.ttf");
    if (QFontDatabase::addApplicationFont(fontPath) == -1) {
        QFontDatabase::addApplicationFont(QStringLiteral("assets/fonts/GoogleSansFlex-VariableFont_GRAD,ROND,opsz,slnt,wdth,wght.ttf"));
    }

    PackageManager* sharedPm = new PackageManager(&app);
    TrayManager::instance()->setPackageManager(sharedPm);

    qmlRegisterSingletonType<ThemeWatcher>("Foundry.Theme", 1, 0, "ThemeWatcher", &ThemeWatcher::create);

    qmlRegisterSingletonType<TrayManager>("Foundry.Tray", 1, 0, "TrayManager", &TrayManager::create);

    qmlRegisterSingletonType<AppImageInstaller>("Foundry.Market", 1, 0, "AppImageInstaller", [](QQmlEngine*, QJSEngine*) -> QObject* {
        return new AppImageInstaller();
    });

    qmlRegisterSingletonType<PackageManager>("Foundry.Market", 1, 0, "PackageManager", [sharedPm](QQmlEngine*, QJSEngine*) -> QObject* {
        return sharedPm;
    });

    const char* configUris[] = { "Foundry.Config", "Caelestia.Config" };
    for (const char* uri : configUris) {
        qmlRegisterSingletonType<caelestia::config::TokenConfig>(uri, 1, 0, "Tokens", &caelestia::config::TokenConfig::create);
        qmlRegisterSingletonType<caelestia::config::GlobalConfig>(uri, 1, 0, "GlobalConfig", &caelestia::config::GlobalConfig::create);

        qmlRegisterUncreatableType<caelestia::config::Tokens>(uri, 1, 0, "Tokens", QStringLiteral("Attached property"));
        qmlRegisterAnonymousType<caelestia::config::AppearanceRounding>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::AppearanceSpacing>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::AppearancePadding>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::AppearanceTransparency>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::AppearanceFont>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::AppearanceAnim>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::AppearanceConfig>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::FontTokens>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::FontStyleBase>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::FontStyle>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::IconFontStyle>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::FontBuilders>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::IconFontBuilders>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::AnimTokens>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::AnimDurationTokens>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::AnimCurves>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::SizeTokens>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::BarTokens>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::AstraTokens>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::DashboardTokens>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::LauncherTokens>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::NotifsTokens>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::OsdTokens>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::SessionTokens>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::SidebarTokens>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::UtilitiesTokens>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::LockTokens>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::WInfoTokens>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::RoundingTokens>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::SpacingTokens>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::PaddingTokens>(uri, 1);
        qmlRegisterAnonymousType<caelestia::config::FontSizeTokens>(uri, 1);
    }

    qmlRegisterType<BlobGroup>("Foundry.Blobs", 1, 0, "BlobGroup");
    qmlRegisterType<BlobInvertedRect>("Foundry.Blobs", 1, 0, "BlobInvertedRect");
    qmlRegisterType<BlobRect>("Foundry.Blobs", 1, 0, "BlobRect");
    qmlRegisterType<BlobMaterial>("Foundry.Blobs", 1, 0, "BlobMaterial");
    qmlRegisterType<BlobShape>("Foundry.Blobs", 1, 0, "BlobShape");

    QQmlApplicationEngine engine;
    engine.addImageProvider(QStringLiteral("icon"), new IconImageProvider());

    // Prefer local filesystem QML if running in development tree, fallback to embedded qrc
    QString localQmlDir;
    if (QFile::exists(QDir::currentPath() + QStringLiteral("/qml/Main.qml"))) {
        localQmlDir = QDir::currentPath() + QStringLiteral("/qml");
    } else if (QFile::exists(QDir::currentPath() + QStringLiteral("/../qml/Main.qml"))) {
        localQmlDir = QFileInfo(QDir::currentPath() + QStringLiteral("/../qml")).canonicalFilePath();
    } else if (QFile::exists(QGuiApplication::applicationDirPath() + QStringLiteral("/../qml/Main.qml"))) {
        localQmlDir = QFileInfo(QGuiApplication::applicationDirPath() + QStringLiteral("/../qml")).canonicalFilePath();
    }

    if (!localQmlDir.isEmpty()) {
        engine.addImportPath(localQmlDir);
        engine.addImportPath(QFileInfo(localQmlDir + QStringLiteral("/..")).canonicalFilePath());
    }
    engine.addImportPath(QStringLiteral("qrc:/qml"));
    engine.addImportPath(QStringLiteral("qrc:/"));

    const QUrl url = !localQmlDir.isEmpty()
        ? QUrl::fromLocalFile(localQmlDir + QStringLiteral("/Main.qml"))
        : QUrl(QStringLiteral("qrc:/qml/Main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject* obj, const QUrl& objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    auto* ipcServer = new QLocalServer(&app);
    ipcServer->setSocketOptions(QLocalServer::UserAccessOption);
    QObject::connect(ipcServer, &QLocalServer::newConnection, ipcServer, [ipcServer]() {
        while (QLocalSocket* client = ipcServer->nextPendingConnection()) {
            QObject::connect(client, &QLocalSocket::disconnected, client, &QLocalSocket::deleteLater);
            QObject::connect(client, &QLocalSocket::readyRead, client, [client]() {
                while (client->canReadLine()) {
                    const QByteArray command = client->readLine().trimmed();
                    if (command == "SHOW" || command == "OPEN_GUI") {
                        TrayManager::instance()->showMainWindow();
                    } else if (command == "TOGGLE") {
                        TrayManager::instance()->toggleMainWindow();
                    } else if (command == "TRAY") {
                        TrayManager::instance()->setTrayEnabled(true);
                    }
                }
            });
        }
    });

    if (!ipcServer->listen(serverName) && ipcServer->serverError() == QAbstractSocket::AddressInUseError) {
        QLocalServer::removeServer(serverName);
        ipcServer->listen(serverName);
    }

    engine.rootContext()->setContextProperty(QStringLiteral("startInTray"), launchTray);
    engine.rootContext()->setContextProperty(QStringLiteral("appVersion"), QStringLiteral(ASTRA_VERSION));
    engine.load(url);

    return app.exec();
}
