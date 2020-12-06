#include "CrashHandler.hpp"

class CrashHandlerImpl
{
public:
    CrashHandlerImpl(void)
    {
    }
};

void CrashHandler::Win64::attach(void)
{
    new CrashHandlerImpl;
}
