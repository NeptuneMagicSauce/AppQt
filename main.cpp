#include <iostream>

#include <QApplication>
#include <QDebug>

#include "LoadContent.hpp"
#include "Logic.hpp"
#include "Gui.hpp"

/* potential names

   Minus
   Ninja Minus
   Rominus
   Minus Is Not Ur Solitaire (s...)

 */


namespace Minus
{
    class App : public QApplication
    {
    public:
        App(int argc, char** argv) :
            QApplication(argc, argv),
            gui(new Gui(logic.width, logic.height))
        {
            auto update_gui = [this] () {
                for (int x=0; x<logic.width; ++x)
                {
                    for (int y=0; y<logic.height; ++y)
                    {
                        gui->addCell(logic.cell(x, y)->widget, y, x);
                    }
                }
                gui->resizeEvent();
            };

            QObject::connect(gui, &Gui::reset, [this, update_gui] () {
                logic.reset(logic.width, logic.height);
                update_gui();
            });

            update_gui();
        }
    private:
        class Loader
        {
        public:
            Loader()
            {
                LoadContent::doLoad();
            }
        } loader;
        Logic logic;
        Gui* gui;
        // gui must be dynamic allocated so that it is not destructed
        // auto destruction on quit fails with silent error
        // maybe because QApplication is being destructed
    };
};


int main(int argc, char **argv)
{
    // qDebug()  << "__cplusplus" << __cplusplus;
    // qDebug() << "__VERSION__" << __VERSION__;

    Minus::App(argc, argv).exec();

    return 0;
}
