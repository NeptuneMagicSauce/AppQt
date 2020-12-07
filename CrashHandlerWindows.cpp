#if _WIN64
#include "CrashHandler.hpp"

#include <windows.h>
#include <imagehlp.h> // SymInitialize() needs linker flag -limagehlp
#include <string>
#include <vector>
#include <iostream>
#include <QDebug>
#include <QProcess>
#include <QCoreApplication>

#include "Utils.hpp"
#include "WindowsErrorCodes.hpp"

using std::string;
using std::vector;

class CrashHandlerImpl
{
public:
    CrashHandlerImpl(void)
    {
        SetUnhandledExceptionFilter(windows_exception_handler);
    }
    static LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS* exception)
    {
        // TODO port FloatingPointExceptions::Disabler
        // FloatingPointExceptions::Disabler fpe;

        // TODO port my assert with line function info
// #define ASSERT(check, message) Assert::Assert((check), (#check), (message), __FILE__, __LINE__, __PRETTY_FUNCTION__)

        // TODO find if we can attach debugger on crash handler
        // does it need a button to trigger another catchable signal ?

        auto ex_code = exception ? exception->ExceptionRecord->ExceptionCode : 0;
        auto error_message =
            Utils::exceptionCode(ex_code) + " " +
            Utils::toHexa(ex_code);

        QStringList stack;
        if (CrashHandler::hasAlreadyCrashed())
        {
            error_message += " double crash";
        } else {
            // TODO build stack trace in background thread maybe ?
            // with QProcess signals
            // also do stack unwalking in the same background thread
            stack = printStackTrace(exception);
            // TODO replace C:/Devel/Workspace/ and C:/Devel/Tools/ with empty
            // TODO split long lines in addr/function/location with start tabs
        }

        std::cerr << error_message << std::endl;
        if (!stack.isEmpty())
        {
            auto debug = qDebug();
            debug.noquote();
            debug.nospace();
            for (const auto& s : stack)
            {
                debug << s << "\n";
            }
        }
        CrashHandler::showDialog(error_message, stack);

        // TODO namespace Utils for classes if not in namespace Minus

        // TODO catch exceptions at top level, they are not caught by CrashHandler

        // TODO what about other threads, do they need to enable these 2:
        // crash handler and floating point exception disabler

        return EXCEPTION_EXECUTE_HANDLER;
    }

    static QStringList addr2line(const vector<void*>& addr)
    {
        // TODO check addr2line is in path
        // TODO xor ship addr2line in application dir with cmake

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
        // return p.readAll();
        return QString(p.readAll()).split("\n", Qt::SkipEmptyParts);
    }

    static QStringList printStackTrace(EXCEPTION_POINTERS* exception)
    {
        QStringList ret;
        ret << "Stack Trace ";
        if (exception == nullptr)
        {
            ret << "exception is null";
            return ret;
        }
        auto* ex_record = exception->ExceptionRecord;
        if (ex_record == nullptr)
        {
            ret << "exception record is null";
            return ret;
        }
        auto* context = exception->ContextRecord;
        if (ex_record->ExceptionCode == DWORD(EXCEPTION_STACK_OVERFLOW))
        {
            if (context == nullptr)
            {
                ret << "context is null";
            return ret;
            }
            // If this is a stack overflow then we can't walk the stack
            // so just show where the error happened

            ret << addr2line({(void*)context->Rip});
            return ret;
        }
        if (context == nullptr)
        {
            ret << "context is null";
            return ret;
        }

        auto handle = GetCurrentProcess();
        SymInitialize(handle, 0, true);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
        STACKFRAME frame = {{ 0 }};
#pragma GCC diagnostic pop
        frame.AddrPC.Offset         = context->Rip;
        frame.AddrPC.Mode           = AddrModeFlat;
        frame.AddrStack.Offset      = context->Rsp;
        frame.AddrStack.Mode        = AddrModeFlat;
        frame.AddrFrame.Offset      = context->Rbp;
        frame.AddrFrame.Mode        = AddrModeFlat;

        vector<void*> addr;
        while (StackWalk(
                   IMAGE_FILE_MACHINE_AMD64,
                   GetCurrentProcess(),
                   GetCurrentThread(),
                   &frame,
                   context,
                   0,
                   SymFunctionTableAccess,
                   SymGetModuleBase,
                   0))
        {
            addr.emplace_back((void*)frame.AddrPC.Offset);
        }
        SymCleanup(GetCurrentProcess());
        ret << addr2line(addr);
        return ret;
    }
};

void CrashHandler::Win64::attach(void)
{
    new CrashHandlerImpl;
}

#endif
