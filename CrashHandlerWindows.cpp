#if _WIN64
#include "CrashHandler.hpp"

#define WIN32_LEAN_AND_MEAN
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
        // TODO port FloatingPointExceptions::Disabler, disable fpe in excpt handler

        // TODO port my assert with line function info
// #define ASSERT(check, message) Assert::Assert((check), (#check), (message), __FILE__, __LINE__, __PRETTY_FUNCTION__)

        // TODO find if we can attach debugger on crash handler
        // does it need a button to trigger another catchable signal ?

        // TODO what about other threads, do they need to enable these 2:
        // crash handler and floating point exception disabler

        auto ex_code = exception ? exception->ExceptionRecord->ExceptionCode : 0;
        auto error_message =
            Utils::exceptionCode(ex_code) + " " +
            Utils::toHexa(ex_code);

        CrashHandler::Stack stack;
        if (CrashHandler::hasAlreadyCrashed())
        {
            error_message += " double crash";
        } else {
            stack = CrashHandler::formatStack(printStackTrace(exception));
        }

        CrashHandler::showTerminal(error_message, stack);
        CrashHandler::showDialog(error_message, stack);

        return EXCEPTION_EXECUTE_HANDLER;
    }

    static QStringList printStackTrace(EXCEPTION_POINTERS* exception)
    {
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

            ret << CrashHandler::addr2line({(void*)context->Rip});
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
        ret << CrashHandler::addr2line(addr);
        return ret;
    }
};

void CrashHandler::Win64::attach(void)
{
    // TODO have launcher be platform specific
    // because Linux gdb will need to be inside a terminal

    // TODO (win64) disable auto handler on attach dbg
    // or tell gdb to continue if breakpoint = ...

    // TODO (win64) ship mintty, gdb with tui, cgdb

    // TODO (win64) hide console on start, show console on crash

    // TODO (win64) ship gdb qt pretty printers and eval-load them ?
    new CrashHandlerImpl;
}

bool CrashHandler::Win64::isDebuggerAttached(void)
{
    return IsDebuggerPresent();
}

void CrashHandler::Win64::breakDebugger(void)
{
    DebugBreak(); // windows api call is safer than asm int3
    // asm volatile ("int3");
}

#endif
