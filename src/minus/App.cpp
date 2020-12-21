#include <iostream>

#include <QDebug>
#include <QTime>

#include "Application.hpp"
#include "logic/Logic.hpp"
#include "gui/Gui.hpp"
#include "Utils.hpp"

/* TODO

   warnings: Wshadow Wshadow-compatible Wshadow-compatible-local

   linter: clang-tidy ?

   better perf with custom render of cells without using widgets
   use a QGraphicsView instead of QFrame ?
   cf https://stackoverflow.com/a/13990849
   and/or cache pixmap of expensive outlined digit on resize?
   QGraphicsView https://doc.qt.io/qt-5/qgraphicsview.html#details
   QQuickPaintedItem https://doc.qt.io/qt-5/qquickpainteditem.html#details
   tuto https://www.bogotobogo.com/Qt/Qt5_QGraphicsView_QGraphicsScene.php

   cmake: needs QT >= 5.14 for QWidget::screen()
   cmake: require custom var QT5_MINGW
   cmake windows: make gnucxx mandatory

   README: description, how to compile :
   mkdir build; cd build; cmake ..; make
   debian/ubuntu: apt install qtbase5-dev g++ make cmake
   windows: install QT dev with option mingw64
   what about make.exe ? should it be cmake --build . ?
   or Tools/Qt5/Tools/mingw810_64/bin/mingw32-make.exe
   add to path g++, moc
   QT5_MINGW: env var or cmake var?


   use QFontMetrics to have perfect vertical alignment

   port Linux
   font size: use QFontMetrics to have it match exactly with windows
   double sigint -> crash of crash handler
   do not catch sigint, also sigterm?
   ctrl-alt-f1 is caught by window manager first
   f5 is not default keybind for refresh
   linux stack trace: portable cstd way, boost:: ? backtrace() ?
   linux check if gdb command continue on attach should be removed
   linux gdb will need to be inside a terminal
   linux check split(\r\n) on output of addr2line
   linux do we have to implement isDebuggerAttached?

   mine ratio: how much is too much? 40% or 50% maybe. do not allow more

   unit tests and code coverage

   setting: ratio, color, emojis, keybinds
   settings: have categories
   save and restore settings

   store files in dirs of src:
   utils
   minus/logic
   minus/gui

   coding style: auto* -> auto

   result dialog : win or lose, stats
   status win/lose: with face emoji

   timer during gameplay in tool bar

   reveal cells with nice animation
   shake viewport
   rebouding particles
   shockwave with cells moving in depth or in XY
   shockwave: will not work with recursion, could work with async signals

   nicer font as baked resource
   outline text of neighbor count
   emoji renders are not nicely anti-aliased
   if sent in random (precomputed) directions

   highlight / specular and/or texture: nice noise or pattern

   do no react on release where pressed but on press
   but only 1 cell per press, no keep pressed for multi cells

   highlight auto revealed cell same as hovered maybe ?

   windows: ship mintty, gdb with tui, cgdb

   windows: hide console on start, show console on crash

   other threads: they need to attach signal handler and disable FPE!

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
            gui(logic.width, logic.height, logic.ratio)
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

            QObject::connect(&gui, &Gui::changeRatio,
                             [this] (float ratio) {
                                 logic.changeRatio(ratio);
                                 emit gui.resetSignal();
                             });

            QObject::connect(&gui, &Gui::changeWidth,
                             [this] (int width) {
                                 logic.changeWidth(width);
                                 emit gui.resetSignal();
                             });

            QObject::connect(&gui, &Gui::changeHeight,
                             [this] (int height) {
                                 logic.changeHeight(height);
                                 emit gui.resetSignal();
                             });

            QObject::connect(&gui, &Gui::resetSignal,
                             [this] () {
                                 logic.reset();
                                 gui.reset();
                             });

            // gui.resizeEvent(); // no longer needed because we do a first resize
        }
    private:
        Logic logic;
        Gui gui;
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
