#include "CellWidget.hpp"
#include "LoadContent.hpp"
#include "Utils.hpp"
#include "Labels.hpp"

#include <random>

#include <QDebug>
#include <QPainter>
#include <QPaintEvent>

using std::vector;

namespace Minus
{
    /* TODO
       highlight hovered cell
       use a QGraphicsView instead of QFrame ?
       cf https://stackoverflow.com/a/13990849
       reveal cells with nice animation
         shake viewport
         rebouding particles
         shockwave with cells moving in depth or in XY
       layouting : square cells, either with window ratio or borders
       layouting : find square cells on un-maximize
       have flags on mouse2
       have auto reveal with flags
       highlight auto revealed cell same as hovered maybe ?
       highlight / specular and/or texture: nice noise or pattern
       do no react on release where pressed but on press
       but only 1 cell per press, no keep pressed for multi cells

       emacs theme light = doom-one-light
       https://github.com/hlissner/emacs-doom-themes/blob/master/themes/doom-one-light-theme.el
     */

    struct CellWidgetImpl: public LoadContent
    {
        CellWidget* cell_of_mouse_press { nullptr };

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

        virtual void loadCallback(void) override
        {
            font.setFamily("Verdana");
            font.setStyleStrategy(QFont::PreferAntialias);
            font.setWeight(QFont::DemiBold); // Medium Bold
        }

        QFont font;
        const QSizePolicy size_policy { QSizePolicy::Expanding, QSizePolicy::Expanding };
        const Qt::Alignment alignment { Qt::AlignHCenter | Qt::AlignVCenter };

    } impl;

    CellWidget::CellWidget(const QColor& color) :
        color(impl.processColor(color)),
        sunken_color(Utils::lerpColor(this->color, Qt::white, 0.25f))
    {
        setAutoFillBackground(true);
        setAlignment(impl.alignment);
        setSizePolicy(impl.size_policy);
        setFont(impl.font);
        raise(Depth::Raised);
    }

    void CellWidget::enable(bool b)
    {
        enabled = b;
        if (enabled == false)
        {
            setText(label);
        }
    }

    void CellWidget::raise(Depth depth)
    {
        setFrameStyle(QFrame::StyledPanel |
                      (depth == Depth::Raised ? QFrame::Raised : QFrame::Sunken));
        setStyleSheet(
            "background-color:" +
            (depth == Depth::Raised ? color : sunken_color).name(QColor::HexRgb) + ";"
            "color:" + label_color.name(QColor::HexRgb) + ";"
            );
    }

    void CellWidget::setLabel(bool mine, int neighbor_mines)
    {
        label =
            mine
            ? Minus::Labels::bomb
            : Minus::Labels::digits[neighbor_mines];
        label_color = Minus::Labels::colors[mine ? 0 : neighbor_mines];
    }

    void CellWidget::mousePressEvent(QMouseEvent *e)
    {
        if (enabled == false)
        {
            return;
        }
        impl.cell_of_mouse_press = this;
        if (e->button() == Qt::LeftButton)
        {
            raise(Depth::Sunken);
        }
    }
    void CellWidget::mouseReleaseEvent(QMouseEvent *e)
    {
        if (enabled == false)
        {
            return;
        }
        const bool released_where_pressed =
            impl.cell_of_mouse_press != nullptr &&
            impl.cell_of_mouse_press->geometry().contains(mapToParent(e->pos()));

        if (e->button() == Qt::LeftButton)
        {
            if (released_where_pressed)
            {
                emit reveal();
            }
            else
            {
                impl.cell_of_mouse_press->raise(Depth::Raised);
            }
        }
        impl.cell_of_mouse_press = nullptr;
    }

};
