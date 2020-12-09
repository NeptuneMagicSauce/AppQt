#pragma once

#include "CrashHandler.hpp"

class CrashDialog
{
public:
    static void panic(const std::string& error, const CrashHandler::Stack& stack={});
    static constexpr const char* prefix_function = "in ";
    static constexpr const char* prefix_location = "at ";
};
