#pragma once

#include <QLabel>


namespace Minus
{
    class CellWidget: public QLabel
    {
    public:
        CellWidget(const QColor&);
        virtual ~CellWidget(void);

        // TODO enum Depth should be private, only external call is for Raised
        enum struct Depth : int { Raised, Sunken };
        void raise(Depth);

        void setLabel(bool mine, int neighbor_mines);
        void setFontSize(int);
        void reveal(void);
        void onPress(void);
        void switchFlag(void);

    public:
        // TODO have these 2 state var private, only getters public
        bool flag { false };
        bool revealed { false };
    private:
        const QColor color, sunken_color;
        QString label;
        QColor label_color { Qt::white };
        float font_size_digit, font_size_bomb;

        friend class CellWidgetImpl; // TODO no longer needed
    };

};
