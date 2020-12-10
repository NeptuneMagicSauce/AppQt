#include "Utils.hpp"

#include <sstream>
#include <set>
#include <cxxabi.h>
#include <QDebug>
#include <QString>

#include "StackTrace.hpp"
#include "CrashDialog.hpp"

using std::string;
using std::ostringstream;
using namespace Utils;

namespace UtilsImpl
{
    void panic(
        const QString& message,
        const QString& file,
        int line,
        const QString& function)
    {
        CrashDialog::panic(
            message,
            true, // can call stack trace
            CrashDialog::Location{ function, file, line });
    }

    char* demangle(const std::type_info& type)
    {
        return abi::__cxa_demangle(type.name(), 0, 0, 0);
    }
};

void Utils::assertSingleton(const std::type_info& type)
{;
    static std::set<string> classes;
    auto class_name = UtilsImpl::demangle(type);
    AssertX(
        !classes.count(string{class_name}),
        "only one instance supported of " + QString{class_name});
    classes.insert(class_name);
}

void Utils::doAssert(
        bool condition,
        const QString& literal,
        const QString& message,
        const QString& file,
        int line,
        const QString& function)
{
    if (condition)
    {
        return;
    }

    auto message_combined = "Assertion not verified: ' " + literal;
    if (message.size())
    {
        message_combined += " '\n\n" + message + "\n";
    }

    UtilsImpl::panic(message_combined, file, line, function);
}


void Utils::panicException(
    const std::exception& e,
    const QString& file,
    int line,
    const QString& function)
{
    UtilsImpl::panic(
        "Unhandled exception: ' " + QString{UtilsImpl::demangle(typeid(e))} +
        "\n' " + e.what() + " '",
        file, line, function);
}
