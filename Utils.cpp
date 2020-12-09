#include "Utils.hpp"

#include <sstream>
#include <set>
#include <cxxabi.h>
#include <QString>

#include "CrashDialog.hpp"

using std::string;
using std::ostringstream;
using namespace Utils;

namespace UtilsImpl
{
    void panic(
        std::ostringstream& error,
        const string& message,
        const string& file,
        int line,
        const string& function)
    {
        if (message.size())
        {
            error << std::endl << message;
        }
        // TODO insert tabs around all lines of error (already has message)
        // TODO massage location with shared function: remove first X dirs
        error
            << std::endl << CrashDialog::prefix_function << function << ""
            << std::endl << CrashDialog::prefix_location << file + ":" + std::to_string(line) << ""
            ;
        CrashDialog::panic(error.str(), CrashHandler::instance().currentStack());
    }

    string demangle(const std::type_info& type)
    {
        return abi::__cxa_demangle(type.name(), 0, 0, 0);
    }
};

void Utils::assertSingleton(const std::type_info& type)
{
    const string class_name = UtilsImpl::demangle(type);
    static std::set<string> classes;
    AssertX(
        !classes.count(class_name),
        "only one instance supported of " + class_name);
    classes.insert(class_name);
}

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
    UtilsImpl::panic(ss, message, file, line, function);
}


void Utils::panicException(
    const std::exception& e,
    const std::string& file,
    int line,
    const std::string& function)
{
    // TODO find stack trace on exception: not at catch but at throw
    std::ostringstream ss;
    ss
        << "Unhandled exception: ' "
        << UtilsImpl::demangle(typeid(e)) << " '"
        << std::endl
        << "' " << e.what() << " '";
    UtilsImpl::panic(ss, "", file, line, function);
}
