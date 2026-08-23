#pragma once

#include <QMap>
#include <QString>
#include <QStringList>
#include <atomic>
#include <functional>
#include <memory>

namespace astra {

class CancellationToken {
public:
    void requestCancellation() { m_cancelled.store(true); }
    bool isCancelled() const { return m_cancelled.load(); }

private:
    std::atomic_bool m_cancelled{false};
};

using CancellationTokenPtr = std::shared_ptr<CancellationToken>;

struct ProcessResult {
    int exitCode{-1};
    bool timedOut{false};
    bool started{false};
    bool cancelled{false};
    QString output;
    QString error;

    bool succeeded() const { return started && !timedOut && !cancelled && exitCode == 0; }
};

using ProcessLineCallback = std::function<void(const QString& line)>;

using ProcessEnvironmentOverrides = QMap<QString, QString>;

ProcessResult runProcess(const QString& program, const QStringList& arguments, int timeoutMs = 15000);
ProcessResult runProcessStreaming(const QString& program, const QStringList& arguments, const ProcessLineCallback& lineCallback, int timeoutMs = 0, const ProcessEnvironmentOverrides& environmentOverrides = {}, const CancellationTokenPtr& cancellation = {});

}
