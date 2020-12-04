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
        void resizeEvent(void);

    signals:
        void reset(void);

    public:
        Frame frame;
    private:
        QMainWindow main_window;
        QToolBar tool_bar;
        void createToolBar(void);

    };
};
