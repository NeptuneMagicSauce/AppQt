#include "Utils.hpp"

#include <QString>

#include "CrashDialog.hpp"

using std::string;
using namespace Utils;

namespace UtilsImpl
{
    string sourceLocation(
        const std::string& file,
        int line,
        const std::string& function="")
    {
        return file + ":" + std::to_string(line) + " " + function;
    }
};

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
    // TODO too much blank space before at
    // TODO format assert: newlines and tabs
    // TODO also better presentation on Dialog
    auto error =
        "assert failed: " + literal + " " +
        message +
        " at " + UtilsImpl::sourceLocation(file, line, function);
    CrashDialog::panic(error, { {
                "",
                function.c_str(),
                UtilsImpl::sourceLocation(file, line).c_str() } });
}
