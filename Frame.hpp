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

    protected:
        virtual void mousePressEvent(QMouseEvent *e) override;
        virtual void mouseReleaseEvent(QMouseEvent *e) override;
        virtual void mouseMoveEvent(QMouseEvent *e) override;

    private:
        class Layout* layout;
        const int& width;
        const int& height;

        CellWidget* widgetOfEvent(QMouseEvent* e);
        void onNewCellPressed(CellWidget* w);
    };

};
