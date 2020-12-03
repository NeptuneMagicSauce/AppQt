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
            m_frame.addCell(widget, row, column);
        }

        void resizeEvent(void);

    signals:
        void reset(void);

    private:

        QMainWindow main_window;
        QToolBar tool_bar;
        Frame m_frame;

        void createToolBar(void);

    public:
        const Frame& frame;
    };
};
