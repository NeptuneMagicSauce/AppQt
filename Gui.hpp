#pragma once

#include <QObject>
#include <QWidget>

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
        void reset_ratio(float ratio);
        void ready(void);

    public:
        // TODO private and friend
        Frame frame;
        QWidget central;
        Utils::SettingsPane settings;
    };
};
