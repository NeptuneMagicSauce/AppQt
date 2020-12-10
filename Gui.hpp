#pragma once

#include <QObject>

#include "Frame.hpp"
#include "SettingsPane.hpp"

namespace Minus
{
    class Gui : public QObject
    {
        Q_OBJECT
    public:
        Gui(const int& width, const int& height);
        void reset(void);
        void resizeEvent(void);

    signals:
        void reset_signal(int width, int height);

    public:
        Frame frame;
        Utils::SettingsPane settings;

    };
};
