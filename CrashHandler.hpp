#pragma once

#include <string>

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
    static bool hasAlreadyCrashed(void);
    static void showDialog(const std::string& error, const std::string& stack);
    static void attach(void)
    {
#if _WIN64
        Win64::attach();
#else
#warning "Crash Handler not implemented"
#endif
    }
};
