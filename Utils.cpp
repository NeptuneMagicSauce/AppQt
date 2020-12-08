#include "Utils.hpp"

using namespace Utils;

void Utils::doAssert(
        bool condition,
        const std::string& literal,
        const std::string& message,
        const std::string& file,
        int line,
        const std::string& function)
{
    if (condition)
    {
        return;
    }

    // TODO build error message, call CrashDialog::Panic
}
