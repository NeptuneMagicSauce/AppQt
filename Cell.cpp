#include "Cell.hpp"

#include <QDebug>
#include <QPainter>
#include <QPaintEvent>

#include "Utils.hpp"

using std::vector;

namespace Minus
{
    /* TODO
       scale font size to window size
       nicer font for digits, add outline
       reveal cells with nice animation
       layouting : square cells, either with window ratio or borders
     */

    struct
    {
        // std::uniform_int_distribution<> distrib { 0, 255 };
        Cell* cell_of_mouse_press { nullptr };
        Cell::RevealCallback revealCallback;
    } state;

    Cell::Cell(const QColor& color) :
        color(color),
        sunken_color(Utils::lerpColor(color, Qt::white, 0.25f))
    {
        setAutoFillBackground(true);
        setStyleSheet("background-color:" + color.name(QColor::HexRgb) +";");
        raise(true);
        setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        // setLineWidth(3); // not respected
    }

    void Cell::setRevealCallback(RevealCallback revealCallback)
    {
        state.revealCallback = revealCallback;
    }

    void Cell::raise(bool raised)
    {
        this->raised = raised;
        setStyleSheet("background-color:" + (raised ? color : sunken_color).name(QColor::HexRgb) +";");
        // TODO test shape = Box, Panel, WinPanel
        setFrameStyle(QFrame::StyledPanel |
                      (raised ? QFrame::Raised : QFrame::Sunken));
        static const auto size(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
        setSizePolicy(size);
    }

    void Cell::mousePressEvent(QMouseEvent *e)
    {
        state.cell_of_mouse_press = this;
        if (e->button() == Qt::LeftButton &&
            revealed == false)
        {
            raise(false);
        }
    }
    void Cell::mouseReleaseEvent(QMouseEvent *e)
    {
        const bool released_where_pressed =
            state.cell_of_mouse_press != nullptr &&
            state.cell_of_mouse_press->geometry().contains(mapToParent(e->pos()));

        if (e->button() == Qt::LeftButton)
        {
            if (released_where_pressed)
            {
                if (revealed == false)
                {
                    state.revealCallback(*this);
                }
            }
            else
            {
                if (state.cell_of_mouse_press->revealed == false)
                {
                    state.cell_of_mouse_press->raise(true);
                }
            }
        }
        state.cell_of_mouse_press = nullptr;
    }

    void Cell::setNeighbors(vector<Cell*>& neighbors)
    {
        this->neighbors.swap(neighbors);
        neighbor_mines = 0;
        for (auto* n: this->neighbors)
        {
            neighbor_mines += n->mine;
        }
    }

};
