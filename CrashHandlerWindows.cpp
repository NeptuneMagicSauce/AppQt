#if _WIN64
#include "CrashHandlerWindows.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <imagehlp.h> // SymInitialize() needs linker flag -limagehlp
#include <string>
#include <vector>
#include <iostream>
#include <QDebug>
#include <QProcess>
#include <QThread>
#include <QCoreApplication>

#include "Utils.hpp"
#include "WindowsErrorCodes.hpp"

using std::string;
using std::vector;

class CrashHandlerWin64Impl
{
public:
    static CrashHandlerWin64* instance;
    static LONG WINAPI windows_exception_handler(EXCEPTION_POINTERS* exception);
    // TODO no state: remove can_attach, cache result
    static bool can_attach_gdb;
};

CrashHandlerWin64* CrashHandlerWin64Impl::instance = nullptr;
bool CrashHandlerWin64Impl::can_attach_gdb = false;

LONG WINAPI CrashHandlerWin64Impl::windows_exception_handler(EXCEPTION_POINTERS* exception)
{
    instance->handle(exception);
    return EXCEPTION_EXECUTE_HANDLER;
}

void CrashHandlerWin64::handle(void* exception_void)
{
    auto* exception = static_cast<EXCEPTION_POINTERS*>(exception_void);

    // TODO ship mintty, gdb with tui, cgdb

    // TODO hide console on start, show console on crash

    // TODO ship gdb qt pretty printers and eval-load them ?

    auto ex_code = exception ? exception->ExceptionRecord->ExceptionCode : 0;
    auto error_message =
        Utils::exceptionCode(ex_code) + " " +
        Utils::toHexa(ex_code);

    CrashHandler::Stack stack;
    if (hasAlreadyCrashed())
    {
        error_message += " double crash";
    } else {
        stack = parseStack(walkStack(exception_void));
    }

    finishPanic(error_message, stack);
}

QStringList CrashHandlerWin64::walkStack(void* exception_void)
{
    auto* exception = static_cast<EXCEPTION_POINTERS*>(exception_void);

    QStringList ret;
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

CrashHandlerWin64::CrashHandlerWin64(void)
{
    CrashHandlerWin64Impl::can_attach_gdb =
        QProcess::startDetached("gdb", { "-q", "-ex", "quit" });
    CrashHandlerWin64Impl::instance = this;
    SetUnhandledExceptionFilter(CrashHandlerWin64Impl::windows_exception_handler);
}

bool CrashHandlerWin64::isDebuggerAttached(void) const
{
    return IsDebuggerPresent();
}

void CrashHandlerWin64::breakDebugger(bool force) const
{
    if (force || isDebuggerAttached())
    {
        DebugBreak(); // windows api call is more portable than asm int3 maybe ?
        // asm volatile ("int3");
    }
}

bool CrashHandlerWin64::canAttachDebugger(void) const
{
    return CrashHandlerWin64Impl::can_attach_gdb;
}

void CrashHandlerWin64::attachDebugger(void) const
{
    auto pid_string = QString::number(QCoreApplication::applicationPid());
    QProcess::startDetached(
        "gdb",
        { "-quiet" ,
          "-ex", "\"attach " + pid_string + "\"",
          // continue on first breakpoint because
          // windows breaks on attach at DbgUiRemoteBreakin
          // TODO linux check if continue command next line should be removed
          // TODO linux gdb will need to be inside a terminal
          "-ex", "continue"
        });

    for (int i=0; i<100; ++i)
    {
        if (isDebuggerAttached())
        {
            breakDebugger();
            break;
        } else {
            if (i > 0)
            {
                qDebug() << "GDB not ready";
            }
            QThread::msleep(100);
        }
    }
}

#endif
