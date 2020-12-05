#pragma once

#include <QLabel>

class CellWidgetImpl;
namespace Minus
{

    class CellWidget: public QLabel
    {
    public:
        CellWidget(const QColor& color=Qt::white);
        virtual ~CellWidget(void);

        void reset(const QColor& color);
        void raise(bool);
        void setLabel(bool mine, int neighbor_mines);
        void setFontSize(int);
        void reveal(void);
        void onPress(void);
        void switchFlag(void);
        void hover(bool);

    public:
        const bool& flag;
        const bool& revealed;
    private:
        bool m_flag = false;
        bool m_revealed = false;
        bool hovered = false;
        QColor color, sunken_color, hovered_color;
        QString label;
        QColor label_color = Qt::white;
        float font_size_digit, font_size_bomb;
        friend class ::CellWidgetImpl;
    };

};
