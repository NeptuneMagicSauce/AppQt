#include "Cell.hpp"

#include <random>

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
       highlight hovered cell
       use a QGraphicsView instead of QFrame ?
       cf https://stackoverflow.com/a/13990849
       reveal cells with nice animation
         shake viewport
         rebouding particles
         shockwave with cells moving in depth or in XY
       layouting : square cells, either with window ratio or borders
       have flags on mouse2
       have auto reveal with flags
       highlight auto revealed cell same as hovered maybe ?
       highlight / specular and/or texture
     */

    struct
    {
        Cell* cell_of_mouse_press { nullptr };
        Cell::RevealCallback revealCallback;

        std::random_device rd;
        std::mt19937 gen { rd() };
        std::uniform_int_distribution<int> distrib{-5, 5};

        QColor processColor(const QColor& color)
        {
            int r, g, b;
            color.getRgb(&r, &g, &b);
            static auto perColor = [this] (int& c)
            {
                c += distrib(gen);
                c = std::max(0, std::min(255, c));
            };
            perColor(r);
            perColor(g);
            perColor(b);
            return QColor(r, g, b);
        }

    } state;

    Cell::Cell(const QColor& color) :
        color(state.processColor(color)),
        sunken_color(Utils::lerpColor(this->color, Qt::white, 0.25f))
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
