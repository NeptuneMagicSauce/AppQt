#pragma once

#include <QApplication>
#include <QTimer>

#include "LoadContent.hpp"

namespace Utils
{
    class Application : public QApplication
    {
    public:
        Application(int argc, char** argv);
        virtual bool notify(QObject *receiver, QEvent *event) override;
    private:
        QTimer cb;
        class Loader
        {
        public:
            Loader()
            {
                Utils::LoadContent::doLoad();
            }
        } loader;
    };
}
