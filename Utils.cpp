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
    string sourceLocation(
        const string& file,
        int line,
        const string& function="")
    {
        return file + ":" + std::to_string(line) + " " + function;
    }
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
        CrashDialog::panic(error.str(), { {
                    "",
                    function.c_str(),
                    UtilsImpl::sourceLocation(file, line).c_str() } });
    }

    string demangle(const std::type_info& type)
    {
        return abi::__cxa_demangle(type.name(), 0, 0, 0);
    }
};

void Utils::assertSingleton(const std::type_info& type)
{
    const std::string class_name = UtilsImpl::demangle(type);
    static std::set<std::string> classes;
    if (classes.count(class_name))
    {
        throw std::runtime_error("only one instance supported of " + class_name);
    }
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
    std::ostringstream ss;
    ss
        << "Unhandled exception: ' "
        << UtilsImpl::demangle(typeid(e)) << " '"
        << std::endl
        << "' " << e.what() << " '";
    UtilsImpl::panic(ss, "", file, line, function);
}
