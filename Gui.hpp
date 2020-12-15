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
        Gui(const int& width, const int& height, const float& ratio);
        void reset(void);
        void resizeEvent(void);

    signals:
        void ready(void);
        void changeWidth(int);
        void changeHeight(int);
        void changeRatio(float);
        void resetSignal(void);

    public:
        // TODO private and friend
        Frame frame;
        QWidget central;
        Utils::SettingsPane settings;
    };
};
