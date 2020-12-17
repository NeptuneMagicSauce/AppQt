#include "CellWidget.hpp"

#include <set>
#include <cmath>
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
        static auto perComponent = [] (int c, int variance)
        {
            return std::clamp(c + int(Utils::randomIndex(2 * variance)) - variance,
                              0, 255);
        };
        // uniform variation in RGB
        /*
        return QColor(
            perComponent(color.red(), 5),
            perComponent(color.green(), 5),
            perComponent(color.blue(), 5));
        */
        // uniform variation in saturation/value
        QColor ret;
        ret.setHsv(
            color.hue(),
            perComponent(color.saturation(), 10),
            perComponent(color.value(), 5));
        return ret;
    }

    static void setColor(CellWidget* w, bool up, bool hovered=false)
    {
        auto bg_color = (up
                         ? (hovered
                            ? w->hovered_color
                            : w->color)
                         : w->sunken_color);
        w->setStyleSheet("background-color:" + bg_color.name(QColor::HexRgb) + ";");
    }

    static QFont customFont(void)
    {
        auto id = QFontDatabase::addApplicationFont(":Cousine-Bold.ttf");
        Assert(id >= 0 && QFontDatabase::applicationFontFamilies(id).size());
        QFont font(QFontDatabase::applicationFontFamilies(id).first());

        font.setStyleStrategy(QFont::StyleStrategy(
                                  QFont::PreferAntialias |
                                  QFont::NoSubpixelAntialias
                                  ));

        return font;
    }
};


CellWidget::LabelOutlined::LabelOutlined(QWidget* parent) :
    QWidget(parent),
    outline(true),
    offsets {
        { -1, -1 },
        { -1, +1 },
        { +1, -1 },
        { +1, +1 } }
{
    all_children << &main;
    for (int i=0; i<offsets.size(); ++i)
    {
        auto child = new QLabel(this);
        child->move(offsets[i]);
        child->setStyleSheet("color: #000000");
        all_children << child;
    }
    for (auto c: all_children)
    {
        c->setParent(this);
        c->setAttribute(Qt::WA_TranslucentBackground);
        c->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void CellWidget::LabelOutlined::setAlignment(Qt::Alignment alignment)
{
    for (auto c: all_children)
    {
        c->setAlignment(alignment);
    }
}

void CellWidget::LabelOutlined::setFont(const QFont& font)
{
    for (auto c: all_children)
    {
        c->setFont(font);
    }
}

void CellWidget::LabelOutlined::show(const Value& value)
{
    main.setStyleSheet("color:" + value.color.name(QColor::HexRgb));
    this->outline = value.outline;
    for (auto c: all_children)
    {
        if (outline || c == &main)
        {
            c->setText(value.text);
        }
    }
    if (isVisible())
    {
        setVisilityChildren();
    } else {
        QWidget::show();
    }
}

void CellWidget::LabelOutlined::setFontSize(int font_size)
{
    auto font = main.font();
    if (font.pointSize() == font_size)
    {
        return;
    }
    font.setPointSize(font_size);
    for (auto c: all_children)
    {
        c->setFont(font);
    }
}

void CellWidget::LabelOutlined::reset(void)
{
    for (auto c: all_children)
    {
        c->setText("");
    }
    hide();
}

void CellWidget::LabelOutlined::resizeEvent(QResizeEvent* e)
{
    for (auto c: all_children)
    {
        c->resize(e->size());
    }
}

void CellWidget::LabelOutlined::showEvent(QShowEvent*)
{
    setVisilityChildren();
}

void CellWidget::LabelOutlined::setVisilityChildren(void)
{
    for (auto c: all_children)
    {
        c->setVisible(outline || (c == &main));
    }
}

CellWidget::CellWidget(const QColor& color) :
    flag(m_flag),
    revealed(m_revealed),
    pushed(m_pushed),
    label_outlined(this)
{
    setMouseTracking(true);
    static auto custom_font = CellWidgetImpl::customFont();
    label_outlined.setAlignment(Qt::AlignCenter);
    label_outlined.setFont(custom_font);

    reset(color);
}

CellWidget::~CellWidget(void)
{
}

void CellWidget::reset(const QColor& c)
{
    m_flag = false;
    m_revealed = false;
    m_pushed = false;
    changeColor(c);
    label_outlined.reset();
    raise(true);
}

void CellWidget::changeColor(const QColor& c)
{
    color = CellWidgetImpl::processColor(c);
    sunken_color = Utils::lerpColor(color, Qt::white, 0.35f);
    hovered_color = Utils::lerpColor(color, Qt::white, 0.18f);
    CellWidgetImpl::setColor(this, !m_revealed);
}

void CellWidget::reveal(void)
{
    raise(false);
    m_revealed = true;

    if (!label.text.isEmpty())
    {
        label_outlined.show(label);
    } else if (flag) {
        label_outlined.hide();
    }
}

void CellWidget::resizeEvent(QResizeEvent* e)
{
    // TODO cache pixmap of expensive outlined digit on resize?
    // or is it only computed once on resize anyway?
    QFrame::resizeEvent(e);
    label_outlined.resize(e->size());
}

void CellWidget::raise(bool up)
{
    m_pushed = !up;
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
    auto digit =
        ((mine == false) && (neighbor_mines > 0))
        ? Minus::Labels::digits[neighbor_mines]
        : "";
    label =
    {
        mine
        ? Minus::Labels::bomb
        : digit,
        Minus::Labels::colors[mine ? 0 : neighbor_mines],
        !digit.isEmpty(),
    };
}

void CellWidget::setFontSize(int font_size)
{
    label_outlined.setFontSize(font_size);
}

void CellWidget::switchFlag(void)
{
    m_flag = !m_flag;
    if (flag)
    {
        label_outlined.show({Labels::flag, Qt::white, false});
    } else {
        label_outlined.hide();
    }
}

void CellWidget::onPress(void)
{
    if (flag == false &&
        revealed == false)
    {
        raise(false);
    }
}

void CellWidget::hover(bool hovered)
{
    if (revealed)
    {
        return;
    }
    CellWidgetImpl::setColor(this, true, hovered);
}
