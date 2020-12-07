#pragma once

#include <string>
#include <QString>

class CrashHandler
{
private:
#if _WIN64
    struct Win64
    {
        static void attach(void);
    };
#endif
public:
    class StackInfo
    {
    public:
        QString address;
        QString function;
        QString location;
    };
    using Stack = QList<StackInfo>;
    static bool hasAlreadyCrashed(void);
    static void showDialog(const std::string& error, const QStringList& stack);
    static void attach(void)
    {
#if _WIN64
        Win64::attach();
#else
#warning "Crash Handler not implemented"
#endif
    }
};
