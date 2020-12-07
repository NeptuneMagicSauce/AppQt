#if _WIN64
#include "CrashHandler.hpp"

#include <windows.h>
#include <imagehlp.h> // SymInitialize() needs linker flag -limagehlp
#include <string>
#include <iostream>
#include <QDebug>

#include "Utils.hpp"
#include "WindowsErrorCodes.hpp"

using std::string;

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

        auto ex_code = exception ? exception->ExceptionRecord->ExceptionCode : 0;
        auto error_message =
            Utils::toHexa(ex_code) + " " +
            Utils::exceptionCode(ex_code) + " " +
            Utils::statusCode(ex_code) + " ";

        std::ostringstream ss;
        ss << error_message << std::endl;
        if (setHasCrashed())
        {
            ss << "double crash";
        } else {
            printStackTrace(exception, ss);
        }

        // TODO custom message box with scroll area for stack trace
        // with bold / color / markdown for easier parsing: same as cgdb
        // and prompt close
        std::cerr << ss.str() << std::endl;

        // TODO namespace Utils for classes if not in namespace Minus

        // TODO catch exceptions at top level, they are not caught by CrashHandler

        return EXCEPTION_EXECUTE_HANDLER;
    }

    static bool setHasCrashed(void)
    {
        static bool has_crashed = false;
        if (has_crashed)
        {
            return true;
        }
        has_crashed = true;
        return false;
    }

    static string addr2line(const std::string& addr)
    {
        // TODO addr2line
        return addr;
    }

    static void printStackTrace(EXCEPTION_POINTERS* exception, std::ostringstream& ss)
    {
        ss << "Stack Trace" << std::endl;
        if (exception == nullptr)
        {
            ss << "exception is null";
            return;
        }
        auto* ex_record = exception->ExceptionRecord;
        if (ex_record == nullptr)
        {
            ss << "exception record is null";
            return;
        }
        auto* context = exception->ContextRecord;
        if (ex_record->ExceptionCode == DWORD(EXCEPTION_STACK_OVERFLOW))
        {
            if (context == nullptr)
            {
                ss << "context is null";
            }
            // If this is a stack overflow then we can't walk the stack
            // so just show where the error happened

            ss << addr2line(Utils::toHexa((void*)context->Rip));
            return;
        }
        if (context == nullptr)
        {
            ss << "context is null";
            return;
        }
        std::ostringstream s_addresses;
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
            auto* address = (void*)frame.AddrPC.Offset;
            s_addresses << Utils::toHexa(address) << " ";
        }
        SymCleanup(GetCurrentProcess());
        ss << addr2line(s_addresses.str()) << std::endl;
    }
};

void CrashHandler::Win64::attach(void)
{
    new CrashHandlerImpl;
}

#endif
