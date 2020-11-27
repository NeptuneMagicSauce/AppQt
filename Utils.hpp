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
};
