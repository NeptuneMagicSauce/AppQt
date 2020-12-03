#pragma once

#include <QMainWindow>
#include <QToolBar>

#include "Frame.hpp"

namespace Minus
{
    class Gui : public QObject
    {
        Q_OBJECT
    public:
        Gui(const int& width, const int& height);

        inline void addCell(CellWidget& widget, int row, int column)
        {
            frame.addCell(widget, row, column);
        }

        void resizeEvent(void);

    signals:
        void reset(void);

    private:

        QMainWindow main_window;
        QToolBar tool_bar;
        Frame frame;

        void createToolBar(void);
    };
};
