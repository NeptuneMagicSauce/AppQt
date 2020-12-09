#include "CrashHandler.hpp"

#include <QDebug>
#include <QProcess>
#include <QCoreApplication>

#include "Utils.hpp"
#include "CrashDialog.hpp"
#if _WIN64
#include "CrashHandlerWindows.hpp"
#endif

using std::string;
using std::vector;

// TODO port FloatingPointExceptions::Disabler, disable fpe in excpt handler

// TODO other threads: do they need to attach crash handler and disable FPE ?

class CrashHandlerNotImplemented: public CrashHandler
{
public:
    virtual bool canAttachDebugger(void) const { return false; }
    virtual bool isDebuggerAttached(void) const { return false; }
    virtual void attachDebugger(void) const { }
    virtual void breakDebugger(bool) const { }
};

namespace CrashHandlerImpl
{
#if _WIN64
    CrashHandler* instance = new CrashHandlerWin64;
#else
#warning "CrashHandler not implemened"
    CrashHandler* instance = new CrashHandlerNotImplemented;
#endif
};

CrashHandler& CrashHandler::instance(void)
{
    Assert(CrashHandlerImpl::instance);
    return *CrashHandlerImpl::instance;
}

bool CrashHandler::hasAlreadyCrashed(void)
{
    static bool has_crashed = false;
    if (has_crashed)
    {
        return true;
    }
    has_crashed = true;
    return false;
}

void CrashHandler::finishPanic(const std::string& error, const Stack& stack)
{
    CrashDialog::panic(error, stack);
}

QStringList CrashHandler::addr2line(const vector<void*>& addr)
{
    QProcess p;
    QStringList args =
        {
            "-C", // --demangle
            // "-s" // --basenames
            "-a", // --addresses
            "-f", // --functions
            "-p", // --pretty-print
            "-e",
            QCoreApplication::applicationFilePath(),
        };
    for (auto& a: addr)
    {
        args << QString::fromStdString(Utils::toHexa(a));
    }

    p.start("addr2line", args);
    if (!p.waitForStarted())
    {
        QStringList ret;
        ret << "addr2line failed";
        for (auto& a: addr)
        {
            ret << QString::fromStdString(Utils::toHexa(a));
        }
        return ret;
    }
    p.waitForFinished();
    // TODO check split(\r\n) on Linux
    return QString(p.readAll()).split("\r\n", Qt::SkipEmptyParts);
}

CrashHandler::Stack CrashHandler::parseStack(const QStringList& stack)
{
    Stack ret;
    for (auto s: stack)
    {
        StackInfo info;

        static auto takeBefore = [] (QString& s, const QString& pattern) {
            auto pattern_len = pattern.size();
            auto len =
                pattern_len
                ? s.indexOf(pattern)
                : s.size();
            if (len == -1)
            {
                auto ret = s;
                s.resize(0);
                return ret.trimmed();
            }
            auto ret = s.left(len).trimmed();
            s.remove(0, len + pattern_len);
            return ret;
        };

        info.address = takeBefore(s, ":");
        info.function = takeBefore(s, " at ");
        info.location = takeBefore(s, "");

        static auto removePrefix = [] (QString& s, const QString& prefix) {
            if (s.startsWith(prefix, Qt::CaseInsensitive))
            {
                s.remove(0, prefix.size());
            }
        };
        removePrefix(info.location, "c:/Devel/Workspace/");
        removePrefix(info.location, "c:/Devel/Tools/");

        ret << info;
    }
    return ret;
}
