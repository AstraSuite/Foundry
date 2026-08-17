#include "pacmanplugin.hpp"
#include <QProcess>
#include <QFile>
#include <QSet>
#include <QDir>

PacmanPlugin::PacmanPlugin(QObject* parent) : IPackagePlugin(parent) {
    m_enabled = m_settings.value(QStringLiteral("Pacman/enabled"), true).toBool();
}

bool PacmanPlugin::isAvailable() const {
    return QFile::exists(QStringLiteral("/usr/bin/pacman"));
}

void PacmanPlugin::setEnabled(bool enabled) {
    if (m_enabled != enabled) {
        m_enabled = enabled;
        m_settings.setValue(QStringLiteral("Pacman/enabled"), enabled);
        emit enabledChanged();
    }
}

QVariantList PacmanPlugin::search(const QString& query, const QVariantMap& options) {
    Q_UNUSED(options);
    QVariantList results;
    if (!isAvailable() || !m_enabled) return results;

    QString q = query.trimmed().toLower();
    if (q.isEmpty()) return results;

    QProcess proc;
    proc.start(QStringLiteral("pacman"), {QStringLiteral("-Ss"), q});
    if (proc.waitForFinished(5000)) {
        QString output = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
        QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        QSet<QString> seen;
        for (int i = 0; i < lines.size(); i += 2) {
            QString header = lines[i];
            QString desc = (i + 1 < lines.size()) ? lines[i + 1].trimmed() : QStringLiteral("");
            QStringList parts = header.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (!parts.isEmpty()) {
                QString fullName = parts.first();
                QString repo = fullName.contains(QLatin1Char('/')) ? fullName.section(QLatin1Char('/'), 0, 0) : QStringLiteral("extra");
                QString pkgName = fullName.contains(QLatin1Char('/')) ? fullName.section(QLatin1Char('/'), 1) : fullName;

                if (seen.contains(pkgName)) continue;
                seen.insert(pkgName);

                QVariantMap item;
                item[QStringLiteral("id")] = pkgName;
                item[QStringLiteral("name")] = pkgName;
                item[QStringLiteral("summary")] = desc;
                item[QStringLiteral("backend")] = QStringLiteral("Pacman");
                item[QStringLiteral("repository")] = repo;
                item[QStringLiteral("scope")] = QStringLiteral("system");
                item[QStringLiteral("icon")] = pkgName;
                results.append(item);
            }
        }
    } else {
        proc.kill();
        proc.waitForFinished(500);
    }
    return results;
}

QVariantList PacmanPlugin::getInstalled() {
    QVariantList results;
    if (!isAvailable() || !m_enabled) return results;

    QSet<QString> localPkgs;
    QProcess qmProc;
    qmProc.start(QStringLiteral("pacman"), {QStringLiteral("-Qm")});
    if (qmProc.waitForFinished(2000)) {
        QString qmOut = QString::fromUtf8(qmProc.readAllStandardOutput()).trimmed();
        QStringList qmLines = qmOut.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (const QString& l : qmLines) {
            QString name = l.split(QLatin1Char(' ')).value(0);
            if (!name.isEmpty()) localPkgs.insert(name);
        }
    } else {
        qmProc.kill();
        qmProc.waitForFinished(500);
    }

    QProcess proc;
    proc.start(QStringLiteral("pacman"), {QStringLiteral("-Qe")});
    if (proc.waitForFinished(5000)) {
        QString output = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
        QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        int count = 0;
        for (const QString& line : lines) {
            if (++count > 200) break;
            QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (parts.size() >= 2) {
                QString pkgId = parts.value(0);
                if (localPkgs.contains(pkgId)) continue;

                QVariantMap item;
                item[QStringLiteral("id")] = pkgId;
                item[QStringLiteral("name")] = pkgId;
                item[QStringLiteral("version")] = parts.value(1);
                item[QStringLiteral("backend")] = QStringLiteral("Pacman");
                item[QStringLiteral("repository")] = QStringLiteral("extra");
                item[QStringLiteral("scope")] = QStringLiteral("system");
                item[QStringLiteral("icon")] = pkgId;
                item[QStringLiteral("isInstalled")] = true;
                results.append(item);
            }
        }
    } else {
        proc.kill();
        proc.waitForFinished(500);
    }
    return results;
}

QVariantList PacmanPlugin::getUpdates() {
    QVariantList results;
    if (!isAvailable() || !m_enabled) return results;

    QProcess proc;
    proc.start(QStringLiteral("checkupdates"));
    if (proc.waitForFinished(8000)) {
        QString output = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
        QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (const QString& line : lines) {
            QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (parts.size() >= 4) {
                QVariantMap item;
                item[QStringLiteral("id")] = parts.value(0);
                item[QStringLiteral("name")] = parts.value(0);
                item[QStringLiteral("version")] = parts.value(1) + QStringLiteral(" -> ") + parts.value(3);
                item[QStringLiteral("backend")] = QStringLiteral("Pacman");
                item[QStringLiteral("scope")] = QStringLiteral("system");
                item[QStringLiteral("icon")] = parts.value(0);
                results.append(item);
            }
        }
    } else {
        proc.kill();
        proc.waitForFinished(500);
    }
    return results;
}

QVariantMap PacmanPlugin::getDetails(const QString& packageId) {
    QVariantMap map;
    map[QStringLiteral("id")] = packageId;
    map[QStringLiteral("name")] = packageId;
    map[QStringLiteral("backend")] = QStringLiteral("Pacman");

    QProcess proc;
    proc.start(QStringLiteral("pacman"), {QStringLiteral("-Si"), packageId});
    if (!proc.waitForFinished(3000) || proc.exitCode() != 0) {
        proc.start(QStringLiteral("pacman"), {QStringLiteral("-Qi"), packageId});
        proc.waitForFinished(3000);
    }

    QString output = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        if (line.startsWith(QLatin1String("Description"))) {
            map[QStringLiteral("summary")] = line.section(QLatin1Char(':'), 1).trimmed();
            map[QStringLiteral("description")] = line.section(QLatin1Char(':'), 1).trimmed();
        } else if (line.startsWith(QLatin1String("Version"))) {
            map[QStringLiteral("version")] = line.section(QLatin1Char(':'), 1).trimmed();
        } else if (line.startsWith(QLatin1String("URL"))) {
            map[QStringLiteral("homepage")] = line.section(QLatin1Char(':'), 1).trimmed();
        } else if (line.startsWith(QLatin1String("Licenses"))) {
            map[QStringLiteral("license")] = line.section(QLatin1Char(':'), 1).trimmed();
        } else if (line.startsWith(QLatin1String("Packager"))) {
            map[QStringLiteral("developer")] = line.section(QLatin1Char(':'), 1).trimmed();
        }
    }
    return map;
}

bool PacmanPlugin::install(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    Q_UNUSED(options);
    if (!isAvailable()) return false;
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start(QStringLiteral("pkexec"), {QStringLiteral("pacman"), QStringLiteral("-S"), QStringLiteral("--noconfirm"), packageId});
    if (!proc.waitForStarted(5000)) return false;

    while (proc.state() == QProcess::Running) {
        if (proc.waitForReadyRead(200)) {
            while (proc.canReadLine()) {
                QString line = QString::fromUtf8(proc.readLine()).trimmed();
                if (!line.isEmpty() && progressCb) progressCb(50, line);
            }
        }
    }
    QString rest = QString::fromUtf8(proc.readAll()).trimmed();
    if (!rest.isEmpty() && progressCb) {
        for (const QString& line : rest.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
            progressCb(100, line.trimmed());
        }
    }
    return proc.exitCode() == 0;
}

bool PacmanPlugin::uninstall(const QString& packageId, const QVariantMap& options, ProgressCallback progressCb) {
    Q_UNUSED(options);
    if (!isAvailable()) return false;
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    proc.start(QStringLiteral("pkexec"), {QStringLiteral("pacman"), QStringLiteral("-Rns"), QStringLiteral("--noconfirm"), packageId});
    if (!proc.waitForStarted(5000)) return false;

    while (proc.state() == QProcess::Running) {
        if (proc.waitForReadyRead(200)) {
            while (proc.canReadLine()) {
                QString line = QString::fromUtf8(proc.readLine()).trimmed();
                if (!line.isEmpty() && progressCb) progressCb(50, line);
            }
        }
    }
    QString rest = QString::fromUtf8(proc.readAll()).trimmed();
    if (!rest.isEmpty() && progressCb) {
        for (const QString& line : rest.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
            progressCb(100, line.trimmed());
        }
    }
    return proc.exitCode() == 0;
}

bool PacmanPlugin::launch(const QString& packageId) {
    if (!isAvailable()) return false;
    return QProcess::startDetached(packageId);
}
