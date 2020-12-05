#include "Frame.hpp"

#include <cmath>
#include <vector>
#include <map>
#include <QGridLayout>
#include <QResizeEvent>
#include <QDebug>

#include "Utils.hpp"
#include "Labels.hpp"

using std::vector;

namespace Minus
{
    class Layout: public QGridLayout
    {
    public:
        Layout(const int& width, const int& height) :
            width(width),
            height(height)
            {
                setContentsMargins(0, 0, 0, 0);
                setSpacing(0);
            }
        virtual void setGeometry(const QRect &r) override
            {
                if (isEmpty())
                {
                    QGridLayout::setGeometry(r);
                    return;
                }

                assert(count() == width * height);

                const auto cell_size = std::min(
                    r.width() / width,
                    r.height() / height);
                const auto
                    start_x = r.width() / 2 - (cell_size * width) / 2,
                    start_y = r.height() / 2 - (cell_size * height) / 2;

                for (int x=0; x<width; ++x)
                {
                    for (int y=0; y<height; ++y)
                    {
                        const QRect rect
                            {
                                start_x + cell_size * x,
                                start_y + cell_size * y,
                                cell_size,
                                cell_size,
                            };
                        itemAtPosition(y, x)->setGeometry(rect);
                    }
                }
            }
        virtual QSize minimumSize() const override
            {
                return QSize { Frame::MinimumCellSize * width, Frame::MinimumCellSize * height };
            }
        const int& width;
        const int& height;
    };

    class Pool
    {
    public:
        void reserve(int count)
        {
            instances.resize(count);
            for (auto& i: instances)
            {
                i = new CellWidget;
            }
        }
        CellWidget* get(void)
        {
            if (index >= instances.size())
            {
                instances.resize(instances.size() + instances.size() / 2);
                for (unsigned int i=index; i<instances.size(); ++i)
                {
                    instances[i] = new CellWidget;
                }
            }
            assert(index < instances.size());
            return instances[index++];
        }
        void reset(void)
        {
            index = 0;
        }

    private:
        std::vector<CellWidget*> instances;
        unsigned int index { 0 };
    };
};

using namespace Minus;

class FrameImpl
{
public:
    Layout* layout;
    CellWidget* cell_pressed { nullptr };
    CellWidget* hovered { nullptr };
    vector<CellWidget*> neighbors_pressed;

    int key_reveal { Qt::Key_S };
    int key_flag { Qt::Key_D };
    bool key_reveal_pressed { false };

    std::map<CellWidget*, Indices> indices;
    vector<vector<CellWidget*>> widgets;
    Pool pool;

    static QColor color(int column, int row, int width, int height)
    {
        constexpr auto max_distance = std::sqrt(2.f);
        static const QColor
            color_min(112, 195, 255),
            color_max(0, 80, 137);
        auto ratio_x = float(column) / (width - 1);
        auto ratio_y = float(row) / (height - 1);
        auto distance =
            std::sqrt(std::pow(ratio_x, 2.f) +
                      std::pow(ratio_y, 2.f))
            / max_distance;
        return Utils::lerpColor(color_min, color_max, distance);
    }

    void raiseAutoNeighbors(void)
    {
        for (auto* n: neighbors_pressed)
        {
            n->raise(true);
        }
        neighbors_pressed.clear();
    }

} impl_f;

Frame::Frame(const int& width, const int& height) :
    width(width),
    height(height)
{
    Utils::assertSingleton(typeid(*this));
    impl_f.layout = new Layout(width, height);
    impl_f.pool.reserve(40 * 40);
    setLayout(impl_f.layout);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    reset();
}

void Frame::reset(void)
{
    while (impl_f.layout->count())
    {
        impl_f.layout->takeAt(0);
    }
    impl_f.cell_pressed = nullptr;
    impl_f.hovered = nullptr;
    impl_f.neighbors_pressed.clear();
    impl_f.indices.clear();
    impl_f.widgets.resize(width);
    for (auto& column: impl_f.widgets)
    {
        column.resize(height);
    }
    impl_f.key_reveal_pressed = false;
    impl_f.pool.reset();
}

void Frame::addCell(int row, int column)
{
    auto* widget = impl_f.pool.get();
    widget->reset(FrameImpl::color(column, row, width, height));
    impl_f.layout->addWidget(widget, row, column);
    impl_f.indices[widget] = { column, row };
    impl_f.widgets[column][row] = widget;
}

void Frame::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (impl_f.indices.empty())
    {
        return;
    }

    // auto scale font size
    const auto size = std::min(
        event->size().width() / width,
        event->size().height() / height)
        * 0.4f; // default was 0.26

    for (auto w : impl_f.indices)
    {
        w.first->setFontSize(size);
    }
}

void Frame::setMineData(const CellStates& data)
{
    for (int x=0; x<width; ++x)
    {
        for (int y=0; y<height; ++y)
        {
            auto& cell = data[x][y];
            impl_f.widgets[x][y]->setLabel(cell.mine, cell.neighbor_mines);
        }
    }
}

void Frame::revealCell(Indices indices)
{
    auto* w = impl_f.widgets[indices.x()][indices.y()];
    w->reveal();
}

void Frame::leaveEvent(QEvent*)
{
    // TODO BUG : right click in frame, move, release right click in tool bar
    // expected : nothing
    // observed : triggers disabled context menu!
    onNewCellPressed(nullptr);
    impl_f.key_reveal_pressed = false;
    if (impl_f.hovered)
    {
        impl_f.hovered->hover(false);
        impl_f.hovered = nullptr;
    }
}

void Frame::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
    if (event->isAutoRepeat()) { return; }
    auto pressing_reveal = event->key() == impl_f.key_reveal;
    impl_f.key_reveal_pressed = pressing_reveal;
    if (pressing_reveal)
    {
        pressEvent(impl_f.hovered, Qt::LeftButton);
    }
}

void Frame::keyReleaseEvent(QKeyEvent *event)
{
    QWidget::keyReleaseEvent(event);
    if (event->isAutoRepeat()) { return; }
    auto releasing_reveal = event->key() == impl_f.key_reveal;
    auto reveal_was_pressed = impl_f.key_reveal_pressed;
    impl_f.key_reveal_pressed &= !releasing_reveal;
    releaseEvent(impl_f.hovered,
                 (reveal_was_pressed && releasing_reveal)
                 ? Qt::LeftButton
                 : (event->key() == impl_f.key_flag)
                 ? Qt::RightButton
                 : Qt::NoButton);
}

void Frame::mousePressEvent(QMouseEvent *e)
{
    pressEvent(widgetOfEvent(e), e->button());
    QWidget::mousePressEvent(e);
}

void Frame::pressEvent(CellWidget* w, int button)
{
    onNewCellPressed(w);
    if (w && button == Qt::LeftButton)
    {
        w->onPress();
    }
}

void Frame::mouseMoveEvent(QMouseEvent *e)
{
    auto* w = widgetOfEvent(e);
    auto pressing_reveal =
        (e->buttons() & Qt::LeftButton) ||
        impl_f.key_reveal_pressed;
    // TODO BUG : press reveal on revealed cell, keep pressed
    // move over unrevealed neighbor
    // move away 1 cell from over unrevealed neighbor
    // expected : unrevealed neighbor is pressed, color pressed
    // observed : unrevealed neighbor is pressed, color unpressed
    // only with keyboard, not with mouse
    // unhover fails when going to tool bar

    if (pressing_reveal)
    {
        onNewCellPressed(w);
    }
    if (w && pressing_reveal)
    {
        w->onPress();
    }
    if (e->buttons() == Qt::NoButton)
    {
        hover(w);
    }
}

void Frame::mouseReleaseEvent(QMouseEvent *e)
{
    releaseEvent(widgetOfEvent(e), e->button());
    QWidget::mouseReleaseEvent(e);
}

void Frame::releaseEvent(CellWidget* w, int button)
{
    impl_f.raiseAutoNeighbors();

    if (w && (button == Qt::LeftButton))
    {
        if (w->revealed)
        {
            emit autoRevealNeighbors(impl_f.indices[w]);
        } else if (w->flag == false)
        {
            emit reveal(impl_f.indices[w]);
        }
    } else if (w && (button == Qt::RightButton))
    {
        if (w->revealed == false)
        {
            w->switchFlag();
            emit setFlag(impl_f.indices[w], w->flag);
        }
    }
    impl_f.cell_pressed = nullptr;
}

CellWidget* Frame::widgetOfEvent(QMouseEvent* e)
{
    return dynamic_cast<CellWidget*>(childAt(e->pos()));
}

void Frame::onNewCellPressed(CellWidget* w)
{
    if (w == impl_f.cell_pressed)
    {
        return;
    }
    if (impl_f.cell_pressed && impl_f.cell_pressed->revealed == false)
    {
        impl_f.cell_pressed->raise(true);
    }
    impl_f.cell_pressed = w;

    impl_f.raiseAutoNeighbors();

    if (w != nullptr && w->revealed == true)
    {
        auto indices = impl_f.indices[w];
        for (int x=indices.x()-1; x<=indices.x()+1; ++x)
        {
            for (int y=indices.y()-1; y<=indices.y()+1; ++y)
            {
                if (x < 0 || y < 0 || x >= width || y >= height || Indices(x,y) == indices)
                {
                    continue;
                }
                auto* n = dynamic_cast<CellWidget*>(impl_f.layout->itemAtPosition(y, x)->widget());
                if (n->revealed == false && n->flag == false)
                {
                    impl_f.neighbors_pressed.emplace_back(n);
                    n->raise(false);
                }
            }
        }
    }
}

void Frame::hover(CellWidget* w)
{
    if (impl_f.hovered == w)
    {
        return;
    }
    if (impl_f.hovered != nullptr)
    {
        impl_f.hovered->hover(false);
    }
    impl_f.hovered = w;
    if (w != nullptr)
    {
        w->hover(true);
    }
}
