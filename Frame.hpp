#pragma once

#include "CellWidget.hpp"

namespace Minus
{
    class Frame: public QWidget
    {
    public:
        Frame(const int& width, const int& height);
        void reset(void);
        void addCell(CellWidget& widget, int row, int column);

        virtual void resizeEvent(QResizeEvent *event) override;

    private:
        class Layout* layout;
        const int& width;
        const int& height;
    };

};
