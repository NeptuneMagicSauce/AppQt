#include <csignal>
#include <QVector>
#include <QDebug>

#include "CrashDialog.hpp"

using namespace Utils;

namespace Utils::CrashHandler
{
    using Type = void*;
    Type install(void);
    Type instance = install();
    void installSignalHandlers(QVector<int> sigs);
    void handler(int signal);
}

void CrashHandler::installSignalHandlers(QVector<int> sigs)
{
    for (auto& s: sigs)
    {
        std::signal(s, CrashHandler::handler);
    }
}

void CrashHandler::handler(int signal)
{
    CrashDialog::panic(QString{"signal "} + (
                       (signal == SIGABRT) ? "ABORT" :
                       (signal == SIGFPE) ? "FPE" :
                       (signal == SIGILL) ? "ILL" :
                       (signal == SIGINT) ? "INT" :
                       (signal == SIGSEGV) ? "SEGV" :
                       (signal == SIGTERM) ? "TERM" :
                       "UNKNOWN"));
}

#if _WIN64
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "WindowsErrorCodes.hpp"
LONG WINAPI  handlerWindows(EXCEPTION_POINTERS* exception)
{
    auto has_code =
        exception != nullptr &&
        exception->ExceptionRecord != nullptr;
    auto ex_code =
        has_code
        ? exception->ExceptionRecord->ExceptionCode
        : 0;
    auto stack_overflow =
        has_code &&
        ex_code == DWORD(EXCEPTION_STACK_OVERFLOW);
    auto message = Utils::exceptionCode(ex_code) + " " + QString::number(ex_code, 16);

    CrashDialog::panic(message, stack_overflow == false);

    return EXCEPTION_EXECUTE_HANDLER;
}
CrashHandler::Type CrashHandler::install(void)
{
    SetUnhandledExceptionFilter(handlerWindows);
    // the windows api handler can not handle these
    // SIGABORT (and all other signals), std::terminate(), throw empty, assert(false)
    installSignalHandlers( {
            // the cstd api handler can not handle these
            // asm "int3", win api DebugBreak()
            // and its stack trace is longer than the winapi
            SIGABRT,
            // SIGSEGV, // this one is already caught by handlerWindows
            // SIGFPE, // this one is already caught by handlerWindows
            SIGINT,
            SIGTERM,
            SIGILL,
        });
    return nullptr;
}
#else
CrashHandler::Type CrashHandler::install(void)
{
    installSignalHandlers( {
            SIGTERM, // termination request, sent to the program
            SIGSEGV, // invalid memory access (segmentation fault)
            SIGINT, // external interrupt, usually initiated by the user
            SIGILL, // invalid program image, such as invalid instruction
            SIGABRT, // abnormal termination condition: std::abort() ...
            SIGFPE, // erroneous arithmetic operation such as divide by zero
        });
    return nullptr;
}
#endif
