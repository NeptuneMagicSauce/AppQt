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
            Utils::exceptionCode(ex_code) + " " +
            Utils::toHexa(ex_code) + " ";

        // TODO work with QString rather than std::string to no have conversions

        std::ostringstream stack_trace_stream;
        if (CrashHandler::hasAlreadyCrashed())
        {
            std::ostringstream message_stream;
            message_stream << error_message << std::endl;
            message_stream << "double crash";
        } else {
            // TODO build stack trace in background thread maybe ?
            printStackTrace(exception, stack_trace_stream);
        }

        std::cerr << error_message << std::endl;
        const auto stack_trace = stack_trace_stream.str();
        if (stack_trace.size())
        {
            std::cerr << stack_trace << std::endl;
        }
        CrashHandler::showDialog(error_message, stack_trace);

        // TODO namespace Utils for classes if not in namespace Minus

        // TODO catch exceptions at top level, they are not caught by CrashHandler

        // TODO what about other threads, do they need to enable these 2:
        // crash handler and floating point exception disabler

        return EXCEPTION_EXECUTE_HANDLER;
    }

    static string addr2line(const std::string& addr)
    {
        // TODO addr2line
        // return addr;
        QString s = addr.c_str();
        QString ret;
        for (auto& a: s.split(' '))
        {
            ret.append(a
                       + " fshjehgilhfdgdshgjkfgjfdsjghshgfdshgjfdlhjgl"
                       + "\n");
        }
        return ret.toStdString();
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
