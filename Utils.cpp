#include "Utils.hpp"

#include <sstream>
#include <set>
#include <cxxabi.h>
#include <QDebug>
#include <QString>

#include "CrashDialog.hpp"

using std::string;
using std::ostringstream;
using namespace Utils;

namespace UtilsImpl
{
    void panic(
        ostringstream& error,
        const string& message,
        const string& file,
        int line,
        const string& function)
    {
        if (message.size())
        {
            error << std::endl << message;
        }

        // TODO work with QString, no conversions from stdString

        CrashDialog::panic(
            error.str(),
            CrashHandler::instance().currentStack(),
            CrashDialog::Location{ function.c_str(), file.c_str(), line });
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

    ostringstream ss;
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
    ostringstream ss;
    ss
        << "Unhandled exception: ' " << UtilsImpl::demangle(typeid(e)) << " '"
        << std::endl
        << "' " << e.what() << " '";
    UtilsImpl::panic(ss, "", file, line, function);
}
