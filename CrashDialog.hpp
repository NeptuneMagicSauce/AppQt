#pragma once

#include <QString>

#include "CrashHandler.hpp"

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
        const CrashHandler::Stack& stack={},
        const Location& location={"", "", 0});
};
