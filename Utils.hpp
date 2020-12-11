#pragma once

#include <QString>
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

    template<class T> auto toHexa(const T &value)
    {
        // std::string style ->
        // std::ostringstream oss;
        // oss << std::hex << value;
        // return oss.str();
        // static constexpr auto ptr_hexa_size = sizeof(intptr_t) * 2;
        const auto hexa_size = sizeof(value) * 2; // 2 chars per byte in hexa
        return
            "0x" +
            QString::number(intptr_t(value), 16).
            rightJustified(hexa_size, '0');
    }

    std::size_t randomIndex(std::size_t size); // return an index between 0 and size-1

    void assertSingleton(const std::type_info& type);

    void doAssert(
        bool condition,
        const QString& literal,
        const QString& message,
        const QString& file,
        int line,
        const QString& function);

    void panicException(
        const std::exception& e,
        const QString& file,
        int line,
        const QString& function);
};

#define Assert(check) Utils::doAssert((check), (#check), "", __FILE__, __LINE__, __PRETTY_FUNCTION__)
#define AssertX(check, message) Utils::doAssert((check), (#check), (message), __FILE__, __LINE__, __PRETTY_FUNCTION__)
#define PanicException(ex) Utils::panicException(ex, __FILE__, __LINE__, __PRETTY_FUNCTION__)
