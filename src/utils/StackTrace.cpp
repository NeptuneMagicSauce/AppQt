#include "StackTrace.hpp"

#include <QVector>
#include <QProcess>
#include <QCoreApplication>

#if _WIN64
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <imagehlp.h> // SymInitialize() needs linker flag -limagehlp
#else
#endif

using namespace Utils;

namespace StackTraceImpl
{
    using Addresses = QVector<void*>;
    StackTrace::Stack parseStack(const Addresses& addresses);
    QStringList addr2line(const Addresses& addresses);
    QString printAddress(void* value)
    {
        constexpr auto pointer_chars = sizeof(void*) * 2;
        return
            "0x" +
            QString::number(intptr_t(value), 16).
            rightJustified(pointer_chars, '0');
    }
#if _WIN64
    Addresses walkStack(CONTEXT* context);
    StackTrace::Stack fromContext(EXCEPTION_POINTERS* exception); // not used
#endif
}

QStringList StackTraceImpl::addr2line(const Addresses& addresses)
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
    QStringList args_addr;
    for (auto& a: addresses)
    {
        args_addr << printAddress(a);
    }
    args << args_addr;

    p.start("addr2line", args);
    if (!p.waitForStarted())
    {
        QStringList ret;
        ret << "addr2line failed";
        ret << args_addr;
        return ret;
    }
    p.waitForFinished();
    auto process_output = QString{ p.readAll() };
    process_output.remove("\r"); // windows carriage return
    return process_output.split("\n", Qt::SkipEmptyParts);
}

StackTrace::Stack StackTraceImpl::parseStack(const StackTraceImpl::Addresses& addresses)
{
    auto addresses_resolved = addr2line(addresses);
    StackTrace::Stack ret;
    for (auto s: addresses_resolved)
    {
        StackTrace::StackInfo info;

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

        ret << info;
    }
    return ret;
}

#if _WIN64
StackTrace::Stack StackTraceImpl::fromContext(EXCEPTION_POINTERS* exception)
{
    if (exception == nullptr)
    {
        return { };
    }
    auto* ex_record = exception->ExceptionRecord;
    if (ex_record == nullptr)
    {
        return { };
    }
    if (ex_record->ExceptionCode == DWORD(EXCEPTION_STACK_OVERFLOW))
    {
        // If this is a stack overflow then we can't walk the stack
        // so just show where the error happened
        // do no try to examine the context
        return { };
    }

    auto context = exception->ContextRecord;
    auto addresses = StackTraceImpl::walkStack(context);
    return StackTraceImpl::parseStack(addresses);
}


StackTrace::Stack StackTrace::getCurrent(void)
{
    auto context = new CONTEXT;
    RtlCaptureContext(context);
    auto addresses = StackTraceImpl::walkStack(context);
    return StackTraceImpl::parseStack(addresses);
}

StackTraceImpl::Addresses StackTraceImpl::walkStack(CONTEXT* context)
{
    if (!context)
    {
        return { };
    }

    auto handle = GetCurrentProcess();
    if (!handle)
    {
        return { };
    }
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

    Addresses ret;
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
        ret << (void*)frame.AddrPC.Offset;
    }
    SymCleanup(GetCurrentProcess());
    return ret;

}
#else
StackTrace::Stack StackTrace::getCurrent(void)
{
    return { };
}
#endif
