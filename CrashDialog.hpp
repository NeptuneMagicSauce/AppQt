#pragma once

#include <QString>

class CrashDialog
{
public:
    struct Location
    {
        QString function;
        QString file;
        int line;
    };
    static void panic(
        const QString& error,
        bool can_call_stack_trace=true,
        const Location& location={"", "", 0});
};
