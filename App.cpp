#include <iostream>

#include <QDebug>

#include "Application.hpp"
#include "Logic.hpp"
#include "Gui.hpp"
#include "Utils.hpp"

/* TODO

   coding style: auto* -> auto
   change assert -> Assert


   better perf if CellWidget herits from Widget not Frame or Label: faster?
   with rendering of pre-rendered textures ?

   port Linux
   linux stack trace: portable cstd way, boost:: ? backtrace() ?
   linux check if gdb command continue on attach should be removed
   linux gdb will need to be inside a terminal
   linux check split(\r\n) on output of addr2line
   linux do we have to implement isDebuggerAttached?

   cmake: do not require custom env var mingw-qt ...
   cmake: store in subdir of build : /bin/

   setting: size, ratio, color, emojis, keybinds
   do not have box layout
   rather, have Settings Pane above Frame
   and maybe darken frame for feedback on modality
   or small part above screen, in corner

   result modal window : win or lose
   status win/lose: with face emoji

   nicer font as baked resource
   outline text of neighbor count
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

   windows: ship mintty, gdb with tui, cgdb

   windows: hide console on start, show console on crash

   check other threads: do they need to attach signal handler and disable FPE ?

   Floating Point Exceptions: forbid inf, nan, allow temporary FPE::Disabler

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

using namespace Utils;

namespace Minus
{

    class App : public Application
    {
    public:
        App(int argc, char** argv) :
            Application(argc, argv),
            gui_ptr(new Gui(logic.width, logic.height)),
            gui(*gui_ptr)
        {

            QObject::connect(&gui.frame, &Frame::reveal,
                             [this] (const Indices& indices) {
                                 logic.reveal(indices);
                             });

            QObject::connect(&gui.frame, &Frame::autoRevealNeighbors,
                             [this] (const Indices& indices) {
                                 logic.autoRevealNeighbors(indices);
                             });

            QObject::connect(&gui.frame, &Frame::setFlag,
                             [this] (const Indices& indices, bool flag) {
                                 logic.setFlag(indices, flag);
                             });

            QObject::connect(&logic, &Logic::setMineData,
                             [this] (const CellStates& data) {
                                 gui.frame.setMineData(data);
                             });

            QObject::connect(&logic, &Logic::setMineRevealed,
                             [this] (const Indices& indices) {
                                 gui.frame.revealCell(indices);
                             });

            QObject::connect(&gui, &Gui::reset_signal, [this] (int w, int h) {
                logic.reset(w, h);
                gui.reset();
            });

            emit gui.reset_signal(logic.width, logic.height);
            // gui.resizeEvent(); // no longer needed because we do a first resize
        }
    private:
        Logic logic;
        Gui* gui_ptr;
        Gui& gui;
        // gui must be dynamic allocated so that it is not destructed
        // because auto destruction on quit fails with silent error
        // maybe because QApplication is being destructed
    };
};


int main(int argc, char **argv)
{
    // qDebug()  << "__cplusplus" << __cplusplus;
    // qDebug() << "__VERSION__" << __VERSION__;
    try
    {
        return Minus::App(argc, argv).exec();
    }catch (std::exception& e) {
        PanicException(e);
    }
    return 1;
}
