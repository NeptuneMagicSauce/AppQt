#pragma once

#include "CrashHandler.hpp"

class CrashDialog
{
public:
    static void panic(const std::string& error, const CrashHandler::Stack& stack={});
};
