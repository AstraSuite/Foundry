#include <QTest>
#include <QElapsedTimer>
#include <QSet>

#include "pluginprocess.hpp"
#include "flatpakplugin.hpp"
#include "pacmanplugin.hpp"

class TestParsers : public QObject {
    Q_OBJECT

private slots:
    void pacmanSearchOutputIsParsed();
    void pacmanSearchKeepsEveryEntry();
    void pacmanInstalledSkipsForeignPackages();
    void pacmanInstalledIsNotTruncated();
    void pacmanUpdatesAreParsed();
    void pacmanInfoParsesWrappedFields();
    void pacmanInfoIgnoresEmptyValues();
    void flatpakInstalledUsesColumns();
    void flatpakInstalledFallsBackToAppId();
    void flatpakUpdatesAreParsed();
    void processReportsExitCode();
    void processRunsWithParsableLocale();
    void processStopsAtTimeout();
    void processStreamsLinesSplitByCarriageReturn();
};

void TestParsers::pacmanSearchOutputIsParsed() {
    const QString output = QStringLiteral(
        "extra/vlc 3.0.21-10 [installed]\n"
        "    Free and open source cross-platform multimedia player\n"
        "multilib/lib32-vlc 3.0.21-1\n"
        "    32-bit libraries\n");

    const QVariantList results = PacmanPlugin::parseSearchOutput(output);
    QCOMPARE(results.size(), 2);

    const QVariantMap first = results.first().toMap();
    QCOMPARE(first.value(QStringLiteral("id")).toString(), QStringLiteral("vlc"));
    QCOMPARE(first.value(QStringLiteral("version")).toString(), QStringLiteral("3.0.21-10"));
    QCOMPARE(first.value(QStringLiteral("repository")).toString(), QStringLiteral("extra"));
    QCOMPARE(first.value(QStringLiteral("summary")).toString(), QStringLiteral("Free and open source cross-platform multimedia player"));
    QVERIFY(first.value(QStringLiteral("isInstalled")).toBool());

    const QVariantMap second = results.last().toMap();
    QCOMPARE(second.value(QStringLiteral("id")).toString(), QStringLiteral("lib32-vlc"));
    QCOMPARE(second.value(QStringLiteral("repository")).toString(), QStringLiteral("multilib"));
    QVERIFY(!second.value(QStringLiteral("isInstalled")).toBool());
}

void TestParsers::pacmanSearchKeepsEveryEntry() {
    QString output;
    for (int i = 0; i < 400; ++i) {
        output += QStringLiteral("extra/package-%1 1.0-1\n    summary %1\n").arg(i);
    }
    QCOMPARE(PacmanPlugin::parseSearchOutput(output).size(), 400);
}

void TestParsers::pacmanInstalledSkipsForeignPackages() {
    const QString output = QStringLiteral("bash 5.3.15-1\nastramarket-git 1.1.0.r0-1\nvlc 3.0.21-10\n");
    const QSet<QString> foreign = PacmanPlugin::parseForeignOutput(QStringLiteral("astramarket-git 1.1.0.r0-1\n"));

    const QVariantList results = PacmanPlugin::parseInstalledOutput(output, foreign);
    QCOMPARE(results.size(), 2);
    QCOMPARE(results.first().toMap().value(QStringLiteral("id")).toString(), QStringLiteral("bash"));
    QCOMPARE(results.first().toMap().value(QStringLiteral("version")).toString(), QStringLiteral("5.3.15-1"));
    QVERIFY(results.first().toMap().value(QStringLiteral("isInstalled")).toBool());
}

void TestParsers::pacmanInstalledIsNotTruncated() {
    QString output;
    for (int i = 0; i < 512; ++i) {
        output += QStringLiteral("package-%1 1.0-1\n").arg(i);
    }
    QCOMPARE(PacmanPlugin::parseInstalledOutput(output, {}).size(), 512);
}

void TestParsers::pacmanUpdatesAreParsed() {
    const QString output = QStringLiteral("linux 7.1.7.arch1-1 -> 7.1.8.arch1-3\nvlc 3.0.21-9 -> 3.0.21-10\n");

    const QVariantList results = PacmanPlugin::parseUpdatesOutput(output);
    QCOMPARE(results.size(), 2);
    QCOMPARE(results.first().toMap().value(QStringLiteral("id")).toString(), QStringLiteral("linux"));
    QCOMPARE(results.first().toMap().value(QStringLiteral("version")).toString(), QStringLiteral("7.1.7.arch1-1 -> 7.1.8.arch1-3"));
}

void TestParsers::pacmanInfoParsesWrappedFields() {
    const QString output = QStringLiteral(
        "Repository      : core\n"
        "Name            : linux\n"
        "Version         : 7.1.8.arch1-3\n"
        "Description     : The Linux kernel\n"
        "URL             : https://github.com/archlinux/linux\n"
        "Licenses        : GPL-2.0-only\n"
        "Optional Deps   : linux-headers: headers and scripts\n"
        "                  linux-firmware: firmware images\n"
        "Installed Size  : 147.72 MiB\n"
        "Packager        : Frederik Schwan <freswa@archlinux.org>\n");

    const QVariantMap details = PacmanPlugin::parseInfoOutput(output, QStringLiteral("linux"));
    QCOMPARE(details.value(QStringLiteral("version")).toString(), QStringLiteral("7.1.8.arch1-3"));
    QCOMPARE(details.value(QStringLiteral("summary")).toString(), QStringLiteral("The Linux kernel"));
    QCOMPARE(details.value(QStringLiteral("homepage")).toString(), QStringLiteral("https://github.com/archlinux/linux"));
    QCOMPARE(details.value(QStringLiteral("license")).toString(), QStringLiteral("GPL-2.0-only"));
    QCOMPARE(details.value(QStringLiteral("developer")).toString(), QStringLiteral("Frederik Schwan <freswa@archlinux.org>"));
    QCOMPARE(details.value(QStringLiteral("repository")).toString(), QStringLiteral("core"));
    QCOMPARE(details.value(QStringLiteral("size")).toString(), QStringLiteral("147.72 MiB"));
}

void TestParsers::pacmanInfoIgnoresEmptyValues() {
    const QString output = QStringLiteral(
        "Name            : bash\n"
        "Version         : 5.3.15-1\n"
        "Groups          : None\n"
        "URL             : None\n");

    const QVariantMap details = PacmanPlugin::parseInfoOutput(output, QStringLiteral("bash"));
    QVERIFY(!details.contains(QStringLiteral("homepage")));
    QCOMPARE(details.value(QStringLiteral("name")).toString(), QStringLiteral("bash"));
}

void TestParsers::flatpakInstalledUsesColumns() {
    const QString output = QStringLiteral("org.videolan.VLC\tVLC\t3.0.21\t312.4 MB\ncom.spotify.Client\tSpotify\t1.2.52\t280.0 MB\n");

    const QVariantList results = FlatpakPlugin::parseInstalledOutput(output, QStringLiteral("system"));
    QCOMPARE(results.size(), 2);

    const QVariantMap first = results.first().toMap();
    QCOMPARE(first.value(QStringLiteral("id")).toString(), QStringLiteral("org.videolan.VLC"));
    QCOMPARE(first.value(QStringLiteral("name")).toString(), QStringLiteral("VLC"));
    QCOMPARE(first.value(QStringLiteral("version")).toString(), QStringLiteral("3.0.21"));
    QCOMPARE(first.value(QStringLiteral("size")).toString(), QStringLiteral("312.4 MB"));
    QCOMPARE(first.value(QStringLiteral("scope")).toString(), QStringLiteral("system"));
}

void TestParsers::flatpakInstalledFallsBackToAppId() {
    const QVariantList results = FlatpakPlugin::parseInstalledOutput(QStringLiteral("org.gnome.Platform\t\t\t\n"), QStringLiteral("user"));
    QCOMPARE(results.size(), 1);
    QCOMPARE(results.first().toMap().value(QStringLiteral("name")).toString(), QStringLiteral("org.gnome.Platform"));
    QCOMPARE(results.first().toMap().value(QStringLiteral("version")).toString(), QStringLiteral("latest"));
}

void TestParsers::flatpakUpdatesAreParsed() {
    const QVariantList results = FlatpakPlugin::parseUpdatesOutput(QStringLiteral("org.freedesktop.Platform.GL.default\tMesa\t26.1.5\n"));
    QCOMPARE(results.size(), 1);
    QCOMPARE(results.first().toMap().value(QStringLiteral("name")).toString(), QStringLiteral("Mesa"));
    QCOMPARE(results.first().toMap().value(QStringLiteral("backend")).toString(), QStringLiteral("Flatpak"));
}

void TestParsers::processReportsExitCode() {
    const astra::ProcessResult ok = astra::runProcess(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), QStringLiteral("printf hello")});
    QVERIFY(ok.succeeded());
    QCOMPARE(ok.output, QStringLiteral("hello"));

    const astra::ProcessResult failed = astra::runProcess(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), QStringLiteral("exit 3")});
    QVERIFY(!failed.succeeded());
    QCOMPARE(failed.exitCode, 3);

    const astra::ProcessResult missing = astra::runProcess(QStringLiteral("astra-does-not-exist"), {});
    QVERIFY(!missing.started);
    QVERIFY(!missing.succeeded());
}

void TestParsers::processRunsWithParsableLocale() {
    const astra::ProcessResult result = astra::runProcess(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), QStringLiteral("printf '%s' \"$LC_ALL\"")});
    QCOMPARE(result.output, QStringLiteral("C.UTF-8"));
}

void TestParsers::processStopsAtTimeout() {
    QElapsedTimer timer;
    timer.start();
    const astra::ProcessResult result = astra::runProcess(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), QStringLiteral("sleep 10")}, 500);
    QVERIFY(result.timedOut);
    QVERIFY(!result.succeeded());
    QVERIFY(timer.elapsed() < 5000);
}

void TestParsers::processStreamsLinesSplitByCarriageReturn() {
    QStringList lines;
    const astra::ProcessResult result = astra::runProcessStreaming(
        QStringLiteral("/bin/sh"),
        {QStringLiteral("-c"), QStringLiteral("printf 'first\\rsecond\\nthird'")},
        [&lines](const QString& line) { lines.append(line); });

    QVERIFY(result.succeeded());
    QCOMPARE(lines, QStringList({QStringLiteral("first"), QStringLiteral("second"), QStringLiteral("third")}));
}

QTEST_GUILESS_MAIN(TestParsers)
#include "tst_parsers.moc"
