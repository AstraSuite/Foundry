#include "pluginprocess.hpp"

#include <QElapsedTimer>
#include <QProcess>
#include <QProcessEnvironment>

namespace astra {

namespace {

constexpr int kStartTimeoutMs = 5000;

QProcessEnvironment parsableEnvironment() {
    static const QProcessEnvironment environment = [] {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert(QStringLiteral("LC_ALL"), QStringLiteral("C.UTF-8"));
        env.insert(QStringLiteral("LANG"), QStringLiteral("C.UTF-8"));
        env.remove(QStringLiteral("LANGUAGE"));
        env.remove(QStringLiteral("LC_MESSAGES"));
        return env;
    }();
    return environment;
}

ProcessResult run(const QString& program, const QStringList& arguments, const ProcessLineCallback& lineCallback, int timeoutMs, bool mergeChannels, const ProcessEnvironmentOverrides& environmentOverrides, const CancellationTokenPtr& cancellation) {
    ProcessResult result;

    QProcessEnvironment environment = parsableEnvironment();
    for (auto it = environmentOverrides.constBegin(); it != environmentOverrides.constEnd(); ++it) {
        environment.insert(it.key(), it.value());
    }

    QProcess process;
    process.setProcessEnvironment(environment);
    if (mergeChannels) process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(program, arguments);

    if (!process.waitForStarted(kStartTimeoutMs)) {
        result.error = process.errorString();
        process.kill();
        process.waitForFinished(1000);
        return result;
    }
    result.started = true;

    QElapsedTimer elapsed;
    elapsed.start();
    QString pending;

    const auto drain = [&](bool flush) {
        const QString chunk = QString::fromUtf8(process.readAllStandardOutput());
        if (!chunk.isEmpty()) {
            result.output += chunk;
            if (lineCallback) pending += chunk;
        }
        if (!mergeChannels) result.error += QString::fromUtf8(process.readAllStandardError());

        if (!lineCallback) return;
        while (true) {
            qsizetype breakAt = -1;
            for (qsizetype i = 0; i < pending.size(); ++i) {
                const QChar character = pending.at(i);
                if (character == QLatin1Char('\n') || character == QLatin1Char('\r')) {
                    breakAt = i;
                    break;
                }
            }
            if (breakAt < 0) break;
            const QString line = pending.left(breakAt).trimmed();
            pending.remove(0, breakAt + 1);
            if (!line.isEmpty()) lineCallback(line);
        }
        if (flush && !pending.isEmpty()) {
            const QString line = pending.trimmed();
            pending.clear();
            if (!line.isEmpty()) lineCallback(line);
        }
    };

    while (process.state() == QProcess::Running) {
        process.waitForReadyRead(200);
        drain(false);

        if (cancellation && cancellation->isCancelled()) {
            result.cancelled = true;
            process.terminate();
            if (!process.waitForFinished(3000)) {
                process.kill();
                process.waitForFinished(1000);
            }
            break;
        }

        if (timeoutMs > 0 && elapsed.hasExpired(timeoutMs)) {
            result.timedOut = true;
            process.kill();
            process.waitForFinished(1000);
            break;
        }
    }

    process.waitForFinished(1000);
    drain(true);

    if (!result.timedOut && !result.cancelled) result.exitCode = process.exitCode();
    return result;
}

}

ProcessResult runProcess(const QString& program, const QStringList& arguments, int timeoutMs) {
    return run(program, arguments, nullptr, timeoutMs, false, {}, {});
}

ProcessResult runProcessStreaming(const QString& program, const QStringList& arguments, const ProcessLineCallback& lineCallback, int timeoutMs, const ProcessEnvironmentOverrides& environmentOverrides, const CancellationTokenPtr& cancellation) {
    return run(program, arguments, lineCallback, timeoutMs, true, environmentOverrides, cancellation);
}

}
