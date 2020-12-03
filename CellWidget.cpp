#include "CellWidget.hpp"
#include "LoadContent.hpp"
#include "Utils.hpp"
#include "Labels.hpp"

#include <random>
#include <set>

#include <QDebug>
#include <QPainter>
#include <QPaintEvent>

using std::set;

namespace Minus
{
    /* TODO

       result modal window : win or lose
       status win/lose: with face emoji

       nicer font as baked resource

       emoji renders are not nicely anti-aliased

       highlight hovered cell
       highlight auto revealed cell same as hovered maybe ?

       allow keyboard for reveal/flag highlighted
       only need to call CellWidget::onRelease(Action)

       use a QGraphicsView instead of QFrame ?
       cf https://stackoverflow.com/a/13990849

       reveal cells with nice animation
         shake viewport
         rebouding particles
         shockwave with cells moving in depth or in XY

       highlight / specular and/or texture: nice noise or pattern

       do no react on release where pressed but on press
       but only 1 cell per press, no keep pressed for multi cells

       emacs theme light = doom-one-light
       https://github.com/hlissner/emacs-doom-themes/blob/master/themes/doom-one-light-theme.el
     */

    struct CellWidgetImpl: public LoadContent
    {
        CellWidget* cell_pressed { nullptr };

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

        CellWidget* widgetOfMouseEvent(CellWidget* w, QMouseEvent* e)
        {
            auto* parent { dynamic_cast<QWidget*>(w->parent()) };
            if (parent == nullptr) { return nullptr; }
            return dynamic_cast<CellWidget*>(parent->childAt(w->mapToParent(e->pos())));
        }

        void onNewCellPressed(CellWidget* w)
        {
            if (instances.count(cell_pressed) == 0)
            {
                cell_pressed = nullptr;
            }
            if (cell_pressed && cell_pressed->revealed == false)
            {
                cell_pressed->raise(CellWidget::Depth::Raised);
            }
            cell_pressed = w;
        }

        virtual void loadCallback(void) override
        {
            font.setFamily("Verdana");
            font.setStyleStrategy(QFont::PreferAntialias);
            font.setWeight(QFont::DemiBold); // Medium Bold
        }

        QFont font;
        const Qt::Alignment alignment { Qt::AlignHCenter | Qt::AlignVCenter };

        set<CellWidget*> instances;
    } impl;

    CellWidget::CellWidget(const QColor& color) :
        revealed(m_revealed),
        flag(m_flag),
        color(impl.processColor(color)),
        sunken_color(Utils::lerpColor(this->color, Qt::white, 0.25f))
    {
        impl.instances.insert(this);
        setAutoFillBackground(true);
        setAlignment(impl.alignment);
        setFont(impl.font);
        raise(Depth::Raised);
    }

    CellWidget::~CellWidget(void)
    {
        impl.instances.erase(this);
    }

    void CellWidget::revealLabel(void)
    {
        m_revealed = true;
        setText(label);
    }

    void CellWidget::raise(Depth depth)
    {
        const auto raised { depth == Depth::Raised };
        setFrameStyle(QFrame::StyledPanel |
                      (raised ? QFrame::Raised : QFrame::Sunken));
        setStyleSheet(
            "background-color:" +
            (raised ? color : sunken_color).name(QColor::HexRgb) + ";"
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
        auto* w { impl.widgetOfMouseEvent(this, e) };
        if (w && e->button() == Qt::LeftButton)
        {
            w->onPress();
        }
    }

    void CellWidget::mouseMoveEvent(QMouseEvent *e)
    {
        auto* w { impl.widgetOfMouseEvent(this, e) };
        if (w == nullptr)
        {
            impl.onNewCellPressed(nullptr);
        } else if (e->buttons() & Qt::LeftButton)
        {
            w->onPress();
        }
    }

    void CellWidget::mouseReleaseEvent(QMouseEvent *e)
    {
        auto* w { impl.widgetOfMouseEvent(this, e) };
        if (w && (e->button() == Qt::LeftButton))
        {
            w->onRelease(Action::Reveal);
        } else if (w && (e->button() == Qt::RightButton))
        {
            w->onRelease(Action::Flag);
        }
    }

    void CellWidget::onPress(void)
    {
        impl.onNewCellPressed(this);

        if (flag == false &&
            revealed == false)
        {
            raise(Depth::Sunken);
        }
    }

    void CellWidget::onRelease(Action action)
    {
        if (action == Action::Reveal)
        {
            if (revealed)
            {
                emit autoRevealNeighbors();
            } else if (flag == false)
            {
                emit reveal();
            }
        } else if (action == Action::Flag)
        {
            if (revealed == false)
            {
                m_flag = !flag;
                // TODO set font size smaller for flag same as mine
                setText(flag ? Labels::flag : "");
            }
        }
        impl.cell_pressed = nullptr;
    }

};
