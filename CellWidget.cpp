#include "CellWidget.hpp"

#include <random>
#include <set>

#include <QDebug>
#include <QMouseEvent>

#include "LoadContent.hpp"
#include "Utils.hpp"
#include "Labels.hpp"

using std::set;

using namespace Minus;

/* TODO

   highlight hovered cell
   highlight auto revealed cell same as hovered maybe ?

   allow keyboard for reveal/flag highlighted
   only need to call CellWidget::onRelease(Action)

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

   highlight / specular and/or texture: nice noise or pattern

   do no react on release where pressed but on press
   but only 1 cell per press, no keep pressed for multi cells

   emacs : show cmake colors in compile buffer
   emacs : respect my identation style

   emacs theme light = doom-one-light
   https://github.com/hlissner/emacs-doom-themes/blob/master/themes/doom-one-light-theme.el
*/

struct CellWidgetImpl: public LoadContent
{
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
    const Qt::Alignment alignment { Qt::AlignHCenter | Qt::AlignVCenter };

} impl_cw;

CellWidget::CellWidget(const QColor& color) :
    flag(m_flag),
    revealed(m_revealed)
{
    setAutoFillBackground(true);
    setAlignment(impl_cw.alignment);
    setFont(impl_cw.font);
    reset(color);
}

CellWidget::~CellWidget(void)
{
}

void CellWidget::reset(const QColor& c)
{
    m_flag = false;
    m_revealed = false;
    color = impl_cw.processColor(c);
    sunken_color = Utils::lerpColor(color, Qt::white, 0.25f);
    setStyleSheet(
        "background-color:" +
        color.name(QColor::HexRgb));
    setText("");
    raise(true);
}

void CellWidget::reveal(void)
{
    raise(false);
    m_revealed = true;
    setText(label);
}

void CellWidget::raise(bool up)
{
    setFrameStyle(QFrame::StyledPanel |
                  (up ? QFrame::Raised : QFrame::Sunken));
    setStyleSheet(
        "background-color:" +
        (up ? color : sunken_color).name(QColor::HexRgb) + ";"
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

void CellWidget::setFontSize(int font_size)
{
    auto f = font();
    if (f.pointSize() != font_size)
    {
        f.setPointSizeF(font_size);
        setFont(f);
    }
}

void CellWidget::switchFlag(void)
{
    m_flag = !m_flag;
    setText(flag ? Labels::flag : "");
}

void CellWidget::onPress(void)
{
    if (m_flag == false &&
        m_revealed == false)
    {
        raise(false);
    }
}
