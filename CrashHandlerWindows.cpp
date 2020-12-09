#if _WIN64
#include "CrashHandlerWindows.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <imagehlp.h> // SymInitialize() needs linker flag -limagehlp
#include <vector>
#include <iostream>
#include <QDebug>
#include <QProcess>
#include <QThread>
#include <QCoreApplication>

#include "Utils.hpp"
#include "WindowsErrorCodes.hpp"

using std::vector;

class CrashHandlerWin64Impl
{
public:
    static CrashHandlerWin64* instance;
    static CONTEXT* contextOfException(EXCEPTION_POINTERS* exception);
    static LONG WINAPI windowsExceptionHandler(EXCEPTION_POINTERS* exception);
};

CrashHandlerWin64* CrashHandlerWin64Impl::instance = nullptr;

LONG WINAPI CrashHandlerWin64Impl::windowsExceptionHandler(EXCEPTION_POINTERS* exception)
{
    instance->handle(exception);
    return EXCEPTION_EXECUTE_HANDLER;
}

void CrashHandlerWin64::handle(void* exception_void) const
{
    auto* exception = static_cast<EXCEPTION_POINTERS*>(exception_void);

    // TODO ship mintty, gdb with tui, cgdb

    // TODO hide console on start, show console on crash

    auto ex_code = exception ? exception->ExceptionRecord->ExceptionCode : 0;
    QString error_message =
        Utils::exceptionCode(ex_code) + " " +
        Utils::toHexa(ex_code);

    CrashHandler::Stack stack;
    if (hasAlreadyCrashed())
    {
        error_message += "\nDouble Crash";
    } else {
        auto* context = CrashHandlerWin64Impl::contextOfException(exception);
        if (context == nullptr)
        {
            error_message += " (No Context)";
        } else {
            stack = parseStack(walkStack(context));
        }
    }

    finishPanic(error_message, stack);
}

CONTEXT* CrashHandlerWin64Impl::contextOfException(EXCEPTION_POINTERS* exception)
{
    if (exception == nullptr)
    {
        return nullptr;
    }
    auto* ex_record = exception->ExceptionRecord;
    if (ex_record == nullptr)
    {
        return nullptr;
    }
    if (ex_record->ExceptionCode == DWORD(EXCEPTION_STACK_OVERFLOW))
    {
        // If this is a stack overflow then we can't walk the stack
        // so just show where the error happened
        // do no try to examine the context
        return nullptr;
    }
    return exception->ContextRecord;
}

QStringList CrashHandlerWin64::walkStack(void* context_void) const
{
    auto* context = static_cast<CONTEXT*>(context_void);

    QStringList ret;
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
    CrashHandlerWin64Impl::instance = this;
    SetUnhandledExceptionFilter(CrashHandlerWin64Impl::windowsExceptionHandler);
}

bool CrashHandlerWin64::isDebuggerAttached(void) const
{
    return IsDebuggerPresent();
}

#if 0
// mini dump can be opened by windbg, maybe its not compatible with gcc -g ?
// it needs cmake: set(CMAKE_CXX_STANDARD_LIBRARIES "-ldbghelp")
void MiniDump(void)
{
    auto h_proc = GetCurrentProcess();
    if (!h_proc)
    {
        qDebug() << "GetCurrentProcess failed";
        return;
    }

    auto path = QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).absoluteFilePath("crash.dmp");
    qDebug() << "CRASHING TO" << path;

    const DWORD Flags = MiniDumpWithFullMemory |
        MiniDumpWithFullMemoryInfo |
        MiniDumpWithHandleData |
        MiniDumpWithUnloadedModules |
        MiniDumpWithThreadInfo;

    auto h_file = CreateFile(path.toUtf8(), GENERIC_ALL, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
   if (!h_file)
   {
       qDebug() << "Failed to write dump: Invalid dump file" << path;
       return;
   }
   auto result = MiniDumpWriteDump( h_proc,
                                    GetProcessId(h_proc),
                                    h_file,
                                    (MINIDUMP_TYPE)Flags,
                                    nullptr,
                                    nullptr,
                                    nullptr );

   CloseHandle(h_file);

   if (!result)
   {
       qDebug() << "MiniDumpWriteDump failed";
   }
}
#endif

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
    static auto ret = QProcess::startDetached("gdb", { "-q", "-ex", "quit" });
    return ret;
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
CrashHandler::Stack CrashHandlerWin64::currentStack(void) const
{
    auto* context = new CONTEXT();
    RtlCaptureContext(context);
    return parseStack(walkStack(context));
}

#endif
