#pragma once

#include <QApplication>
#include <QTimer>

#include "LoadContent.hpp"

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
            LoadContent::doLoad();
        }
    } loader;
};
