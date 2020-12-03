#pragma once

#include <QLabel>


namespace Minus
{
    class CellWidget: public QLabel
    {
    public:
        CellWidget(const QColor&);
        virtual ~CellWidget(void);

        enum struct Depth : int { Raised, Sunken };
        void raise(Depth);
        void revealLabel(void);
        void setLabel(bool mine, int neighbor_mines);
        void setFontSize(int);

    public:
        bool flag { false };
        bool revealed { false };
    private:
        QString label;
        const QColor color, sunken_color;
        QColor label_color { Qt::white };
        float font_size_digit, font_size_bomb;

    public:
        void onPress(void);
        void switchFlag(void);

        friend class CellWidgetImpl;
    };

};
