#include <array>
#include <QString>

namespace Minus::Labels
{
    const std::array<QString, 10> digits
    {
        "",
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
    };
    const QString bomb = "💣";
    const QString flag = "🚩";
    const QString reset = "😅";
    const QString settings = "⚙️";

    const std::array<QColor, 10> colors
    {
        QColor(Qt::white),

        // QColor("#F94144"),
        // QColor("#F3722C"),
        // QColor("#F8961E"),
        // QColor("#F9844A"),
        // QColor("#F9C74F"),
        // QColor("#90BE6D"),
        // QColor("#43AA8B"),
        // QColor("#4D908E"),
        // QColor("#577590"),

        QColor("#FA0C07"),
        QColor("#4CFA07"),
        QColor("#FFBE0B"),
        QColor("#8338EC"),
        QColor("#00FFBB"),
        QColor("#84ED39"),
        QColor("#FFD43B"),
        QColor("#FF6C0A"),
        QColor("#FF00C3"),
    };
};
