#include "CellWidget.hpp"

#include <random>
#include <set>

#include <QTimer>
#include <QDebug>
#include <QMouseEvent>

#include "LoadContent.hpp"
#include "Utils.hpp"
#include "Labels.hpp"

using std::set;

using namespace Utils;
using namespace Minus;

struct CellWidgetImpl: public LoadContent
{
    std::random_device rd;
    std::mt19937 gen { rd() };
    std::uniform_int_distribution<int> distrib {-5, 5};

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

    void setColor(CellWidget* w, bool up, bool hovered=false)
    {
        auto bg_color = (up
                         ? (hovered
                            ? w->hovered_color
                            : w->color)
                         : w->sunken_color);
        w->setStyleSheet(
            "background-color:" + bg_color.name(QColor::HexRgb) + ";"
            "color:" + w->label_color.name(QColor::HexRgb) + ";"
            );
    }

    virtual void loadCallback(void) override
    {
        font.setFamily("Verdana");
        font.setStyleStrategy(QFont::PreferAntialias);
        font.setWeight(QFont::DemiBold); // Medium Bold
    }

    QFont font;

} impl_cw;

CellWidget::CellWidget(const QColor& color) :
    flag(m_flag),
    revealed(m_revealed)
{
    setMouseTracking(true);
    setAutoFillBackground(true);
    setAlignment(Qt::AlignCenter);
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
    hovered = false;
    color = impl_cw.processColor(c);
    sunken_color = Utils::lerpColor(color, Qt::white, 0.25f);
    hovered_color = Utils::lerpColor(color, Qt::white, 0.18f);
    impl_cw.setColor(this, true);
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
    setFrameStyle(
        // QFrame::StyledPanel | // respect qApp->style
        // QFrame::Box |         // ugly
        QFrame::WinPanel |    // deep raised/sunken difference
        // QFrame::Panel |       // soft difference
                  (up ? QFrame::Raised : QFrame::Sunken));
    impl_cw.setColor(this, up);
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
    if (flag == false &&
        revealed == false)
    {
        raise(false);
    }
}

void CellWidget::hover(bool h)
{
    if (revealed)
    {
        return;
    }
    hovered = h;
    impl_cw.setColor(this, true, hovered);
}
