#pragma once

#include <QString>
#include <QStringList>
#include <functional>

namespace astra {

struct ProcessResult {
    int exitCode{-1};
    bool timedOut{false};
    bool started{false};
    QString output;
    QString error;

    bool succeeded() const { return started && !timedOut && exitCode == 0; }
};

using ProcessLineCallback = std::function<void(const QString& line)>;

ProcessResult runProcess(const QString& program, const QStringList& arguments, int timeoutMs = 15000);
ProcessResult runProcessStreaming(const QString& program, const QStringList& arguments, const ProcessLineCallback& lineCallback, int timeoutMs = 0);

}
