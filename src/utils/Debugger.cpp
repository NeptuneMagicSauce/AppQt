#include "Debugger.hpp"

#include <fstream>
#include <QString>
#include <QCoreApplication>
#include <QProcess>
#include <QThread>
#include <QDebug>

using namespace Utils;

namespace DebuggerImpl
{
    bool hasCGDB()
    {
#if _WIN64
        return false;
#elif __linux__
        QProcess p;
        p.start("which", { "cgdb" });
        return
            p.waitForStarted() &&
            p.waitForFinished() &&
            p.exitCode() == 0;
#endif
    }
};

bool Debugger::canAttachDebugger(void)
{
    static auto ret = QProcess::startDetached("gdb", { "-q", "-ex", "quit" });
    if (ret == 0)
    {
        return false;
    }
#if __linux__
    try
    {
        auto path = "/proc/sys/kernel/yama/ptrace_scope";
        std::ifstream proc{path};
        int ptrace_scope = -1;
        proc >> ptrace_scope;
        if (ptrace_scope > 0)
        {
            qDebug() << "Can not attach debugger with value 1 in" << path;
            qDebug() << "Consider putting 0 there or in /etc/sysctl.d/10-ptrace.conf";
            return false;
        }
    } catch (std::exception&) { }
#endif
    return true;
}

void Debugger::attachDebugger(void)
{
    auto pid_string = QString::number(QCoreApplication::applicationPid());

    static auto hasCGDB = DebuggerImpl::hasCGDB();

    auto command =
        hasCGDB
        ? "cgdb"
        : "gdb";
    auto parameterTUI =
        hasCGDB
        ? ""
        : "--tui";

    QProcess::startDetached(
        command,
        { "-quiet" ,
          "-ex",
#if _WIN64
          "\"attach " + pid_string + "\"",
          // continue on first breakpoint because
          // windows breaks on attach at DbgUiRemoteBreakin
          "-ex", "continue"
#else
          "attach " + pid_string,
          parameterTUI
#endif
        });

    // wait for gdb attach
#if _WIN64
#warning "test if linux impl also works on windows"
    for (int i=20; i>=0; --i)
    {
        if (isDebuggerAttached())
        {
            breakDebugger();
            break;
        } else
        {
            qDebug() << "waiting for GDB attach" << i;
            QThread::msleep(100);
        }
    }
#else
    QThread::msleep(2 * 1000); // 2 seconds
#endif
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
#else // not windows = linux
void Debugger::breakDebugger(void)
{
    asm volatile ("int3");
}
bool Debugger::isDebuggerAttached(void)
{
    try
    {
        std::ifstream proc("/proc/self/status");
        std::string word;
        while (proc >> word)
        {
            if (word == "TracerPid:")
            {
                int pid;
                proc >> pid;
                return pid != 0;
            }
        }
    } catch (std::exception const&) { }
    return false;
}
#endif
