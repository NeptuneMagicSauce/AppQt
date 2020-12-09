#pragma once

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
        const std::string& error,
        const CrashHandler::Stack& stack={},
        const Location& location={"", "", 0});
};
