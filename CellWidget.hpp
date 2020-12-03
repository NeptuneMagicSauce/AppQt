#pragma once

#include <QLabel>

namespace Minus
{
    class CellWidget: public QLabel
    {
    public:
        CellWidget(const QColor&);
        virtual ~CellWidget(void);

        void pushUp(void) { raise(true); }
        void setLabel(bool mine, int neighbor_mines);
        void setFontSize(int);
        void reveal(void);
        void onPress(void);
        void switchFlag(void);

    public:
        const bool& flag;
        const bool& revealed;
    private:
        bool m_flag { false };
        bool m_revealed { false };
        const QColor color, sunken_color;
        QString label;
        QColor label_color { Qt::white };
        float font_size_digit, font_size_bomb;
        void raise(bool);
    };

};
