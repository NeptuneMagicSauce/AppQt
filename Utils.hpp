#pragma once

#include <string>
#include <sstream>
#include <set>
#include <cxxabi.h>
#include <QColor>

namespace Utils
{
    inline float lerp(float a, float b, float t)
    {
        return a + t * (b - a);
    }
    inline QColor lerpColor(const QColor& a, const QColor& b, float t)
    {
        float
            red = lerp(a.redF(), b.redF(), t),
            green = lerp(a.greenF(), b.greenF(), t),
            blue = lerp(a.blueF(), b.blueF(), t);
        return QColor(int(red * 255), int(green * 255), int(blue * 255));
    }

    inline void assertSingleton(const std::type_info& type)
    {
        const std::string class_name = abi::__cxa_demangle(type.name(), 0, 0, 0);
        static std::set<std::string> classes;
        if (classes.count(class_name))
        {
            throw std::runtime_error("only one instance supported of " + class_name);
        }
        classes.insert(class_name);
    }

    template<class T> std::string toHexa(const T &value)
    {
        std::ostringstream oss;
        oss << std::hex << value;
        return oss.str();
    }

#define Assert(check, message) doAssert((check), (#check), (message), __FILE__, __LINE__, __PRETTY_FUNCTION__)
    void doAssert(
        bool condition,
        const std::string& literal,
        const std::string& message,
        const std::string& file,
        int line,
        const std::string& function);
};
