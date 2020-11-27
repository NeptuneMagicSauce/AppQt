#include "Cell.h"

#include <QDebug>
#include <QPainter>
#include <QPaintEvent>

namespace Minus
{
    /* TODO
       base color : blue or green or ...
       background color : ratio of distance to origin
       revealed color : something else, light gray
       move mouse outside of cell without release => raise
       mouse release inside of cell = reveal IF press was inside of same cell
       print neighbor count or mine


     */

    std::mt19937* Cell::gen { nullptr };
    // std::uniform_real_distribution<float> Cell::distrib { 0.f, 1.f };
    std::uniform_int_distribution<int> Cell::distrib { 0, 255 };

    Cell::Cell() :
        color(distrib(*gen), distrib(*gen), distrib(*gen))
    {
        setAutoFillBackground(true);
        setStyleSheet("background-color:" + color.name(QColor::HexRgb) +";");
        raise(true);
        // setLineWidth(3); // not respected
    }

    void Cell::raise(bool raised)
    {
        setFrameStyle(QFrame::StyledPanel |
                      (raised ? QFrame::Raised : QFrame::Sunken));
    }

    void Cell::mousePressEvent(QMouseEvent *e)
    {
        if (e->button() == Qt::LeftButton)
        {
            raise(false);
        }
    }

};
