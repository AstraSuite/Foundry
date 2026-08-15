#include "themewatcher.hpp"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QStandardPaths>
#include <QDebug>

ThemeWatcher::ThemeWatcher(QObject* parent)
    : QObject(parent) {
    const QString filePath = getSchemeFilePath();
    const QFileInfo fi(filePath);
    if (!fi.dir().exists()) {
        fi.dir().mkpath(QStringLiteral("."));
    }

    if (QFile::exists(filePath)) {
        m_watcher.addPath(filePath);
    } else {

        m_watcher.addPath(fi.dir().path());
    }

    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, [this](const QString& path) {
        if (!m_watcher.files().contains(path) && QFile::exists(path)) {
            m_watcher.addPath(path);
        }
        reload();
    });
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [this](const QString&) {
        const QString path = getSchemeFilePath();
        if (QFile::exists(path) && !m_watcher.files().contains(path)) {
            m_watcher.addPath(path);
        }
        reload();
    });

    reload();
}

void ThemeWatcher::setSyncScheme(bool sync) {
    if (m_syncScheme != sync) {
        m_syncScheme = sync;
        emit syncSchemeChanged();
        reload();
    }
}

void ThemeWatcher::setSyncTokens(bool sync) {
    if (m_syncTokens != sync) {
        m_syncTokens = sync;
        emit syncTokensChanged();
        reload();
    }
}

QString ThemeWatcher::getSchemeFilePath() const {
    const QString stateHome = qEnvironmentVariable("XDG_STATE_HOME", QDir::homePath() + QStringLiteral("/.local/state"));
    return stateHome + QStringLiteral("/caelestia/scheme.json");
}

void ThemeWatcher::applyFallbackColours() {
    m_name = QStringLiteral("astramarket");
    m_mode = QStringLiteral("dark");
    m_flavour = QStringLiteral("default");
    m_variant = QStringLiteral("content");

    m_colours = QVariantMap{
        { QStringLiteral("background"), QStringLiteral("0a0f0f") },
        { QStringLiteral("surface"), QStringLiteral("0a0f0f") },
        { QStringLiteral("surfaceDim"), QStringLiteral("0a0f0f") },
        { QStringLiteral("surfaceBright"), QStringLiteral("242e2d") },
        { QStringLiteral("surfaceContainerLowest"), QStringLiteral("000000") },
        { QStringLiteral("surfaceContainerLow"), QStringLiteral("0e1514") },
        { QStringLiteral("surfaceContainer"), QStringLiteral("131b1a") },
        { QStringLiteral("surfaceContainerHigh"), QStringLiteral("192120") },
        { QStringLiteral("surfaceContainerHighest"), QStringLiteral("1d2827") },
        { QStringLiteral("onSurface"), QStringLiteral("dce8e6") },
        { QStringLiteral("onBackground"), QStringLiteral("dce8e6") },
        { QStringLiteral("surfaceVariant"), QStringLiteral("1d2827") },
        { QStringLiteral("onSurfaceVariant"), QStringLiteral("a2adac") },
        { QStringLiteral("outline"), QStringLiteral("6d7876") },
        { QStringLiteral("outlineVariant"), QStringLiteral("3f4a49") },
        { QStringLiteral("primary"), QStringLiteral("9bd0cc") },
        { QStringLiteral("onPrimary"), QStringLiteral("0d4845") },
        { QStringLiteral("primaryContainer"), QStringLiteral("255b58") },
        { QStringLiteral("onPrimaryContainer"), QStringLiteral("b8ede9") },
        { QStringLiteral("secondary"), QStringLiteral("b0ccc9") },
        { QStringLiteral("onSecondary"), QStringLiteral("2c4543") },
        { QStringLiteral("secondaryContainer"), QStringLiteral("27403e") },
        { QStringLiteral("onSecondaryContainer"), QStringLiteral("a9c5c2") },
        { QStringLiteral("tertiary"), QStringLiteral("d5efff") },
        { QStringLiteral("onTertiary"), QStringLiteral("2e5c72") },
        { QStringLiteral("tertiaryContainer"), QStringLiteral("b6e3fe") },
        { QStringLiteral("onTertiaryContainer"), QStringLiteral("255369") },
        { QStringLiteral("error"), QStringLiteral("fa746f") },
        { QStringLiteral("onError"), QStringLiteral("490006") },
        { QStringLiteral("errorContainer"), QStringLiteral("871f21") },
        { QStringLiteral("onErrorContainer"), QStringLiteral("ff9993") }
    };
    emit themeChanged();
}

void ThemeWatcher::reload() {
    const QString path = getSchemeFilePath();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        applyFallbackColours();
        return;
    }

    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseErr;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "ThemeWatcher: Failed to parse scheme JSON:" << parseErr.errorString();
        return;
    }

    const QJsonObject root = doc.object();
    m_name = root.value(QStringLiteral("name")).toString(m_name);
    m_flavour = root.value(QStringLiteral("flavour")).toString(m_flavour);
    m_mode = root.value(QStringLiteral("mode")).toString(m_mode);
    m_variant = root.value(QStringLiteral("variant")).toString(m_variant);

    if (root.contains(QStringLiteral("colours")) && root.value(QStringLiteral("colours")).isObject()) {
        m_colours = root.value(QStringLiteral("colours")).toObject().toVariantMap();
    }

    emit themeChanged();
}

void ThemeWatcher::setMode(const QString& mode) {
    if (!QStandardPaths::findExecutable(QStringLiteral("caelestia")).isEmpty()) {
        QProcess::startDetached(QStringLiteral("caelestia"), { QStringLiteral("scheme"), QStringLiteral("set"), QStringLiteral("--notify"), QStringLiteral("-m"), mode });
    } else {
        m_mode = mode;
        emit themeChanged();
    }
}

void ThemeWatcher::setScheme(const QString& name, const QString& flavour) {
    if (!QStandardPaths::findExecutable(QStringLiteral("caelestia")).isEmpty()) {
        QProcess::startDetached(QStringLiteral("caelestia"), { QStringLiteral("scheme"), QStringLiteral("set"), QStringLiteral("-n"), name, QStringLiteral("-f"), flavour });
    } else {
        m_name = name;
        m_flavour = flavour;
        emit themeChanged();
    }
}

void ThemeWatcher::setVariant(const QString& variant) {
    if (!QStandardPaths::findExecutable(QStringLiteral("caelestia")).isEmpty()) {
        QProcess::startDetached(QStringLiteral("caelestia"), { QStringLiteral("scheme"), QStringLiteral("set"), QStringLiteral("-v"), variant });
    } else {
        m_variant = variant;
        emit themeChanged();
    }
}

QVariantList ThemeWatcher::getSchemes() {
    QProcess proc;
    proc.start(QStringLiteral("caelestia"), { QStringLiteral("scheme"), QStringLiteral("list") });
    if (!proc.waitForFinished(3000)) {
        return {};
    }

    const QByteArray out = proc.readAllStandardOutput();
    const QJsonDocument doc = QJsonDocument::fromJson(out);
    if (!doc.isObject())
        return {};

    QVariantList list;
    const QJsonObject obj = doc.object();
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        const QString sName = it.key();
        if (it.value().isObject()) {
            const QJsonObject flavours = it.value().toObject();
            for (auto fit = flavours.constBegin(); fit != flavours.constEnd(); ++fit) {
                const QString sFlavour = fit.key();
                QVariantMap item;
                item[QStringLiteral("name")] = sName;
                item[QStringLiteral("flavour")] = sFlavour;
                item[QStringLiteral("colours")] = fit.value().toObject().toVariantMap();
                list.append(item);
            }
        }
    }
    return list;
}
