#include "Debugger.hpp"

#include <QString>
#include <QCoreApplication>
#include <QProcess>
#include <QThread>
#include <QDebug>

using namespace Utils;

bool Debugger::canAttachDebugger(void)
{
    static auto ret = QProcess::startDetached("gdb", { "-q", "-ex", "quit" });
    return ret;
}

void Debugger::attachDebugger(void)
{
    auto pid_string = QString::number(QCoreApplication::applicationPid());
    QProcess::startDetached(
        "gdb",
        { "-quiet" ,
          "-ex", "\"attach " + pid_string + "\"",
          // continue on first breakpoint because
          // windows breaks on attach at DbgUiRemoteBreakin
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

#if _WIN64
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
void Debugger::breakDebugger(void)
{
    DebugBreak();
    // asm volatile ("int3");
}
bool Debugger::isDebuggerAttached(void)
{
    return IsDebuggerPresent();
}
#if 0
// mini dump can be opened by windbg, but maybe its not compatible with gcc -g ?
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
#else
#warning "Debugger not implemened"
void Debugger::breakDebugger(void)
{
    asm volatile ("int3");
}
bool Debugger::isDebuggerAttached(void)
{
    return false;
}
#endif
