#include "CellWidget.hpp"

#include <set>

#include <QTimer>
#include <QDebug>
#include <QMouseEvent>
#include <QFontDatabase>

#include "Utils.hpp"
#include "Labels.hpp"

using std::set;

using namespace Utils;
using namespace Minus;

class CellWidgetImpl
{
public:
    static QColor processColor(const QColor& color)
    {
        int r, g, b;
        color.getRgb(&r, &g, &b);
        static auto perColor = [] (int& c)
        {
            // TODO random variation of color amplitude depends on hue sat var
            c += Utils::randomIndex(10) - 5;
            c = std::max(0, std::min(255, c));
        };
        perColor(r);
        perColor(g);
        perColor(b);
        return QColor(r, g, b);
    }

    static void setColor(CellWidget* w, bool up, bool hovered=false)
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

    static QFont customFont(void)
    {
        auto id = QFontDatabase::addApplicationFont(":Cousine-Bold.ttf");
        Assert(id >= 0 && QFontDatabase::applicationFontFamilies(id).size());
        QFont font(QFontDatabase::applicationFontFamilies(id).first());
        font.setStyleStrategy(QFont::PreferAntialias);
        return font;
    }
};

CellWidget::CellWidget(const QColor& color) :
    flag(m_flag),
    revealed(m_revealed)
{
    setMouseTracking(true);
    setAlignment(Qt::AlignCenter);
    static auto custom_font = CellWidgetImpl::customFont();
    setFont(custom_font);
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
    changeColor(c);
    setText("");
    raise(true);
}

void CellWidget::changeColor(const QColor& c)
{
    color = CellWidgetImpl::processColor(c);
    sunken_color = Utils::lerpColor(color, Qt::white, 0.25f);
    hovered_color = Utils::lerpColor(color, Qt::white, 0.18f);
    CellWidgetImpl::setColor(this, true);
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
    CellWidgetImpl::setColor(this, up);
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
    CellWidgetImpl::setColor(this, true, hovered);
}
