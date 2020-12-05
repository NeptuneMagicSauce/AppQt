#include <iostream>

#include <QApplication>
#include <QDebug>

#include "LoadContent.hpp"
#include "Logic.hpp"
#include "Gui.hpp"

/* TODO

   setting: size, ratio, color, emojis, keybinds

   result modal window : win or lose
   status win/lose: with face emoji

   nicer font as baked resource

   emoji renders are not nicely anti-aliased

   use a QGraphicsView instead of QFrame ?
   cf https://stackoverflow.com/a/13990849

   reveal cells with nice animation
   shake viewport
   rebouding particles
   shockwave with cells moving in depth or in XY
   shockwave: will not work with recursion, could work with async signals
   if sent in random (precomputed) directions

   highlight / specular and/or texture: nice noise or pattern

   do no react on release where pressed but on press
   but only 1 cell per press, no keep pressed for multi cells

   highlight auto revealed cell same as hovered maybe ?

   emacs : save on compile
   emacs : show cmake colors in compile buffer
   emacs : respect my identation style

   see how OSS does autowin first reveal: gnome-mines, kmines

   emacs theme light = doom-one-light
   https://github.com/hlissner/emacs-doom-themes/blob/master/themes/doom-one-light-theme.el
*/

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

            QObject::connect(&gui->frame, &Frame::reveal,
                             [this] (const Indices& indices) {
                                 logic.reveal(indices);
                             });

            QObject::connect(&gui->frame, &Frame::autoRevealNeighbors,
                             [this] (const Indices& indices) {
                                 logic.autoRevealNeighbors(indices);
                             });

            QObject::connect(&gui->frame, &Frame::setFlag,
                             [this] (const Indices& indices, bool flag) {
                                 logic.setFlag(indices, flag);
                             });

            QObject::connect(&logic, &Logic::setMineData,
                             [this] (const CellStates& data) {
                                 gui->frame.setMineData(data);
                             });

            QObject::connect(&logic, &Logic::setMineRevealed,
                             [this] (const Indices& indices) {
                                 gui->frame.revealCell(indices);
                             });

            auto update_gui = [this] () {
                for (int x=0; x<logic.width; ++x)
                {
                    for (int y=0; y<logic.height; ++y)
                    {
                        gui->frame.addCell(y, x);
                    }
                }
                gui->resizeEvent();
            };

            QObject::connect(gui, &Gui::reset,
                             [this, update_gui] () {
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
