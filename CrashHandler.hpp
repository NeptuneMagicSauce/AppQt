#pragma once

#include <string>
#include <vector>
#include <QString>

class CrashHandler
{
private:
#if _WIN64
    struct Win64
    {
        static void attach(void);
        static bool isDebuggerAttached(void);
        static void breakDebugger(void);
    };
#endif
public:
    class StackInfo
    {
    public:
        QString address;
        QString function;
        QString location;
        QString prettyPrint(bool has_horizontal_scroll=false, bool rich_text=false) const;
    };
    using Stack = QList<StackInfo>;

    static QStringList addr2line(const std::vector<void*>& addr);
    static Stack formatStack(const QStringList& stack);
    static bool hasAlreadyCrashed(void);

    static void showDialog(const std::string& error, const Stack& stack);
    static void showTerminal(const std::string& error, const Stack& stack);

    // TODO interface and implementor
    static void attach(void)
    {
#if _WIN64
        Win64::attach();
#else
#warning "Crash Handler not implemented"
#endif
    }
    static bool isDebuggerAttached(void)
    {
#if _WIN64
        return Win64::isDebuggerAttached();
#else
#warning "isDebuggerAttached not implemented"
        return false;
#endif
    }
    static void breakDebugger(void)
    {
        if (isDebuggerAttached() == false)
        {
            return;
        }
#if _WIN64
        Win64::breakDebugger();
#else
#warning "breakDebugger not implemented"
#endif
    }
};
