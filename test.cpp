#include <iostream>

#include <QApplication>

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
    class App
    {
        // TODO inherit from QApplication, call LoadContent before MainWindow
    public:
        Logic logic;
        Gui gui;
        App() :
            gui(logic.width, logic.height)
        {
            QObject::connect(&gui, &Gui::reset, [this] () {
                logic.reset(logic.width, logic.height);
                for (int x=0; x<logic.width; ++x)
                {
                    for (int y=0; y<logic.height; ++y)
                    {
                        gui.addCell(logic.cell(x, y)->widget, y, x);
                    }
                }
                gui.resizeEvent();
            });

            emit gui.reset();
        }
    };
};


int main(int argc, char **argv)
{
    // std::cout << "__cplusplus " << __cplusplus << std::endl;
    // std::cout << "__VERSION__ " << __VERSION__ << std::endl;

    QApplication q_app(argc, argv);

    LoadContent::doLoad();

    Minus::App app;

    q_app.exec();

    return 0;
}
