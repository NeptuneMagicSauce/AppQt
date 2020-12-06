#pragma once

#include <QApplication>
#include <QTimer>

#include "LoadContent.hpp"

class Application : public QApplication
{
public:
    Application(int argc, char** argv);
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
