#pragma once

#include <QObject>

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
        void showSettings(bool);

    public:
        Frame frame;

    };
};
