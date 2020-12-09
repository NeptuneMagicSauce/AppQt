#pragma once

#include <string>
#include <sstream>
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

    template<class T> std::string toHexa(const T &value)
    {
        std::ostringstream oss;
        oss << std::hex << value;
        return oss.str();
    }

    void assertSingleton(const std::type_info& type);

    void doAssert(
        bool condition,
        const std::string& literal,
        const std::string& message,
        const std::string& file,
        int line,
        const std::string& function);

    void panicException(
        const std::exception& e,
        const std::string& file,
        int line,
        const std::string& function);
};

#define Assert(check) Utils::doAssert((check), (#check), "", __FILE__, __LINE__, __PRETTY_FUNCTION__)
#define AssertX(check, message) Utils::doAssert((check), (#check), (message), __FILE__, __LINE__, __PRETTY_FUNCTION__)
#define PanicException(ex) Utils::panicException(ex, __FILE__, __LINE__, __PRETTY_FUNCTION__)
