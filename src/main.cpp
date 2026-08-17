#include <QGuiApplication>
#include <QSurfaceFormat>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFontDatabase>
#include <QIcon>
#include <QDir>
#include <QCommandLineParser>
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

#include <QQuickImageProvider>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QSvgRenderer>
#include <QTextStream>

class QtIconThemeImageProvider : public QQuickImageProvider {
public:
    QtIconThemeImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override {
        int w = (requestedSize.width() > 0) ? requestedSize.width() : 128;
        int h = (requestedSize.height() > 0) ? requestedSize.height() : 128;
        QSize targetSize(w, h);

        QString name = id;
        if (name.contains(QLatin1Char('?'))) {
            name = name.section(QLatin1Char('?'), 0, 0);
        }
        if (name.startsWith(QLatin1String("image://icon/"))) {
            name = name.mid(13);
        }

        if (name.isEmpty()) return QImage();

        auto loadIconFile = [&targetSize, size](const QString& path) -> QImage {
            if (!QFile::exists(path)) return QImage();
            if (path.endsWith(QLatin1String(".svg"), Qt::CaseInsensitive)) {
                QSvgRenderer renderer(path);
                if (renderer.isValid()) {
                    QImage img(targetSize, QImage::Format_ARGB32_Premultiplied);
                    img.fill(Qt::transparent);
                    QPainter painter(&img);
                    renderer.render(&painter);
                    if (size) *size = img.size();
                    return img;
                }
            } else {
                QImage img(path);
                if (!img.isNull()) {
                    if (size) *size = img.size();
                    return img;
                }
            }
            return QImage();
        };

        QString theme = QIcon::themeName();
        if (theme.isEmpty() || theme == QStringLiteral("hicolor")) {
            theme = QStringLiteral("Papirus-Dark");
        }

        QStringList searchDirs = {
            QDir::homePath() + QStringLiteral("/.local/share/icons/") + theme,
            QStringLiteral("/usr/share/icons/") + theme,
            QDir::homePath() + QStringLiteral("/.local/share/icons/Papirus-Dark"),
            QStringLiteral("/usr/share/icons/Papirus-Dark"),
            QDir::homePath() + QStringLiteral("/.local/share/icons/Papirus"),
            QStringLiteral("/usr/share/icons/Papirus"),
            QDir::homePath() + QStringLiteral("/.local/share/icons/breeze-dark"),
            QStringLiteral("/usr/share/icons/breeze-dark"),
            QDir::homePath() + QStringLiteral("/.local/share/icons/breeze"),
            QStringLiteral("/usr/share/icons/breeze"),
            QDir::homePath() + QStringLiteral("/.local/share/icons/Adwaita"),
            QStringLiteral("/usr/share/icons/Adwaita"),
            QDir::homePath() + QStringLiteral("/.local/share/flatpak/exports/share/icons/hicolor"),
            QStringLiteral("/var/lib/flatpak/exports/share/icons/hicolor"),
            QDir::homePath() + QStringLiteral("/.local/share/icons/hicolor"),
            QStringLiteral("/usr/share/icons/hicolor"),
            QDir::homePath() + QStringLiteral("/.local/share/icons"),
            QStringLiteral("/usr/share/pixmaps")
        };
        static const QStringList subDirs = {
            QStringLiteral("scalable/apps"),
            QStringLiteral("512x512/apps"),
            QStringLiteral("256x256/apps"),
            QStringLiteral("128x128/apps"),
            QStringLiteral("64x64/apps"),
            QStringLiteral("48x48/apps"),
            QStringLiteral("32x32/apps"),
            QStringLiteral("symbolic/apps"),
            QStringLiteral("apps"),
            QStringLiteral("scalable/categories"),
            QStringLiteral("128x128/categories"),
            QStringLiteral("64x64/categories"),
            QStringLiteral("48x48/categories"),
            QStringLiteral("")
        };
        static const QStringList exts = {
            QStringLiteral(".svg"),
            QStringLiteral(".png"),
            QStringLiteral(".Flatpak.svg"),
            QStringLiteral(".xpm")
        };

        QStringList namesToTry = { name, name.toLower() };
        if (name.contains(QLatin1Char('.'))) {
            namesToTry.append(name.section(QLatin1Char('.'), -1));
        }
        namesToTry.removeDuplicates();

        for (const QString& n : namesToTry) {
            for (const QString& baseDir : searchDirs) {
                for (const QString& subDir : subDirs) {
                    for (const QString& ext : exts) {
                        QString path = subDir.isEmpty() ? baseDir + QStringLiteral("/") + n + ext : baseDir + QStringLiteral("/") + subDir + QStringLiteral("/") + n + ext;
                        QImage img = loadIconFile(path);
                        if (!img.isNull()) {
                            return img;
                        }
                    }
                }
            }
        }

        static const QStringList appDirs = {
            QStringLiteral("/usr/share/applications"),
            QDir::homePath() + QStringLiteral("/.local/share/applications"),
            QDir::homePath() + QStringLiteral("/.local/share/flatpak/exports/share/applications"),
            QStringLiteral("/var/lib/flatpak/exports/share/applications")
        };
        for (const QString& appDir : appDirs) {
            QDir dir(appDir);
            if (!dir.exists()) continue;
            QStringList desktopFiles = dir.entryList({QStringLiteral("*.desktop")}, QDir::Files);
            for (const QString& df : desktopFiles) {
                bool matches = false;
                for (const QString& n : namesToTry) {
                    if (df.contains(n, Qt::CaseInsensitive)) {
                        matches = true;
                        break;
                    }
                }
                if (matches) {
                    QFile file(dir.filePath(df));
                    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                        QTextStream in(&file);
                        while (!in.atEnd()) {
                            QString line = in.readLine().trimmed();
                            if (line.startsWith(QLatin1String("Icon="))) {
                                QString val = line.mid(5).trimmed();
                                if (!val.isEmpty()) {
                                    if (val.startsWith(QLatin1Char('/'))) {
                                        QImage img = loadIconFile(val);
                                        if (!img.isNull()) {
                                            return img;
                                        }
                                    } else {
                                        for (const QString& baseDir : searchDirs) {
                                            for (const QString& subDir : subDirs) {
                                                for (const QString& ext : exts) {
                                                    QString path = subDir.isEmpty() ? baseDir + QStringLiteral("/") + val + ext : baseDir + QStringLiteral("/") + subDir + QStringLiteral("/") + val + ext;
                                                    QImage img = loadIconFile(path);
                                                    if (!img.isNull()) {
                                                        return img;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        return QImage();
    }
};

#include "cli/clihandler.hpp"
#include "tray/traymanager.hpp"
#include <QApplication>
#include <QQmlContext>
#include <QQuickWindow>
#include <QLocalServer>
#include <QLocalSocket>

int main(int argc, char* argv[]) {
    qputenv("QML_XHR_ALLOW_FILE_READ", "1");
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
    const QString serverName = QStringLiteral("astra-market-single-instance-") + userName;

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

    QSurfaceFormat format;
    format.setRedBufferSize(8);
    format.setGreenBufferSize(8);
    format.setBlueBufferSize(8);
    format.setAlphaBufferSize(8);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);

#ifndef ASTRA_VERSION
#define ASTRA_VERSION "1.1.0"
#endif

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("astra"));
    app.setOrganizationName(QStringLiteral("AstraMarket"));
    app.setDesktopFileName(QStringLiteral("astra"));
    app.setApplicationVersion(QStringLiteral(ASTRA_VERSION));
    app.setQuitOnLastWindowClosed(false);

    if (!launchGui) {
        PackageManager pm;
        return CliHandler::run(argc, argv, pm);
    }

    const QString iconPath = QStringLiteral(":/assets/icons/AstraMarket.svg");
    if (QFile::exists(iconPath)) {
        app.setWindowIcon(QIcon(iconPath));
    } else {
        app.setWindowIcon(QIcon(QStringLiteral("assets/icons/AstraMarket.svg")));
    }

    const QString fontPath = QStringLiteral(":/assets/fonts/GoogleSansFlex-VariableFont_GRAD,ROND,opsz,slnt,wdth,wght.ttf");
    if (QFontDatabase::addApplicationFont(fontPath) == -1) {
        QFontDatabase::addApplicationFont(QStringLiteral("assets/fonts/GoogleSansFlex-VariableFont_GRAD,ROND,opsz,slnt,wdth,wght.ttf"));
    }

    PackageManager* sharedPm = new PackageManager(&app);
    TrayManager::instance()->setPackageManager(sharedPm);

    qmlRegisterSingletonType<ThemeWatcher>("AstraMarket.Theme", 1, 0, "ThemeWatcher", &ThemeWatcher::create);

    qmlRegisterSingletonType<TrayManager>("AstraMarket.Tray", 1, 0, "TrayManager", &TrayManager::create);

    qmlRegisterSingletonType<AppImageInstaller>("AstraMarket.Market", 1, 0, "AppImageInstaller", [](QQmlEngine*, QJSEngine*) -> QObject* {
        return new AppImageInstaller();
    });

    qmlRegisterSingletonType<PackageManager>("AstraMarket.Market", 1, 0, "PackageManager", [sharedPm](QQmlEngine*, QJSEngine*) -> QObject* {
        return sharedPm;
    });

    const char* configUris[] = { "AstraMarket.Config", "Caelestia.Config" };
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

    qmlRegisterType<BlobGroup>("AstraMarket.Blobs", 1, 0, "BlobGroup");
    qmlRegisterType<BlobInvertedRect>("AstraMarket.Blobs", 1, 0, "BlobInvertedRect");
    qmlRegisterType<BlobRect>("AstraMarket.Blobs", 1, 0, "BlobRect");
    qmlRegisterType<BlobMaterial>("AstraMarket.Blobs", 1, 0, "BlobMaterial");
    qmlRegisterType<BlobShape>("AstraMarket.Blobs", 1, 0, "BlobShape");

    QQmlApplicationEngine engine;
    engine.addImageProvider(QStringLiteral("icon"), new QtIconThemeImageProvider());

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

    QObject::connect(TrayManager::instance(), &TrayManager::requestShowMainWindow, [&engine]() {
        const auto rootObjects = engine.rootObjects();
        for (auto* obj : rootObjects) {
            if (auto* window = qobject_cast<QQuickWindow*>(obj)) {
                window->setVisible(true);
                window->show();
                window->raise();
                window->requestActivate();
            }
        }
    });

    QObject::connect(TrayManager::instance(), &TrayManager::requestToggleMainWindow, [&engine]() {
        const auto rootObjects = engine.rootObjects();
        for (auto* obj : rootObjects) {
            if (auto* window = qobject_cast<QQuickWindow*>(obj)) {
                if (window->isVisible()) {
                    window->setVisible(false);
                } else {
                    window->setVisible(true);
                    window->show();
                    window->raise();
                    window->requestActivate();
                }
            }
        }
    });

    QLocalServer::removeServer(serverName);
    auto* ipcServer = new QLocalServer(&app);
    QObject::connect(ipcServer, &QLocalServer::newConnection, [ipcServer]() {
        while (QLocalSocket* client = ipcServer->nextPendingConnection()) {
            QObject::connect(client, &QLocalSocket::readyRead, [client]() {
                const QByteArray cmd = client->readAll().trimmed();
                if (cmd == "SHOW" || cmd == "OPEN_GUI") {
                    TrayManager::instance()->showMainWindow();
                } else if (cmd == "TOGGLE") {
                    TrayManager::instance()->toggleMainWindow();
                }
            });
        }
    });
    ipcServer->listen(serverName);

    engine.rootContext()->setContextProperty(QStringLiteral("startInTray"), launchTray);
    engine.rootContext()->setContextProperty(QStringLiteral("appVersion"), QStringLiteral(ASTRA_VERSION));
    engine.load(url);

    return app.exec();
}
