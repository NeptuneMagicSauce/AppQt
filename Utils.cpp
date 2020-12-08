#include "Utils.hpp"

#include <sstream>
#include <QString>

#include "CrashDialog.hpp"

using std::string;
using namespace Utils;

namespace UtilsImpl
{
    string sourceLocation(
        const string& file,
        int line,
        const string& function="")
    {
        return file + ":" + std::to_string(line) + " " + function;
    }
};

void Utils::doAssert(
        bool condition,
        const string& literal,
        const string& message,
        const string& file,
        int line,
        const string& function)
{
    if (condition)
    {
        return;
    }

    std::ostringstream ss;
    ss << "Assertion not verified: ' " << literal << " '";
    if (message.size())
    {
        ss << std::endl << message;
    }
    CrashDialog::panic(ss.str(), { {
                "",
                function.c_str(),
                UtilsImpl::sourceLocation(file, line).c_str() } });
}
