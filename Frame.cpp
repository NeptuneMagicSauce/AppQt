#include "Frame.hpp"

#include <cmath>
#include <vector>
#include <map>
#include <QGridLayout>
#include <QResizeEvent>
#include <QDebug>

#include "Utils.hpp"
#include "Labels.hpp"
#include "Pool.hpp"

using std::vector;
using namespace Utils;

namespace Minus
{
    class Layout: public QGridLayout
    {
    public:
        Layout(const int& width, const int& height, int max_width, int max_height) :
            width(width),
            height(height),
            max_width(max_width),
            max_height(max_height)
        {
            setContentsMargins(0, 0, 0, 0);
            setSpacing(0);
            pool.reserve(max_width * max_height);
            for (int column=0; column<max_width; ++column)
            {
                for (int row=0; row<max_height; ++row)
                {
                    auto w = pool.get();
                    addWidget(w, row, column);
                    w->setVisible(false);
                }
            }
        }
        virtual void setGeometry(const QRect &r) override
        {
            QGridLayout::setGeometry(r);
            if (isEmpty())
            {
                return;
            }

            Assert(width <= max_width && height <= max_height);
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

        void reset(void)
        {
            Assert(width <= max_width && height <= max_height);
            for (int column=0; column<columnCount(); ++column)
            {
                for (int row=0; row<rowCount(); ++row)
                {
                    itemAtPosition(row, column)->widget()->setVisible(false);
                }
            }
            pool.reset();
        }

    private:
        const int& width;
        const int& height;
        const int max_width, max_height;
        Pool<CellWidget> pool;
    };
};

using namespace Minus;

class FrameImpl
{
public:
    Frame* frame = nullptr;
    Layout* layout = nullptr;
    CellWidget* cell_pressed = nullptr;
    CellWidget* hovered = nullptr;
    CellWidget* under_mouse = nullptr;
    vector<CellWidget*> neighbors_pressed;

    int key_reveal = Qt::Key_S;
    int key_flag = Qt::Key_D;
    bool key_reveal_pressed = false;

    std::map<CellWidget*, Indices> indices;
    vector<vector<CellWidget*>> widgets;
    vector<vector<vector<Indices>>> neighbors;

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

    void reset(int widh, int height);
    void onCellPressed(CellWidget* w);
    void pressEvent(CellWidget* w, int button);
    void releaseEvent(CellWidget* w, int button);
    void hover(CellWidget* w);

} impl_f;

Frame::Frame(
    const int& width,
    const int& height,
    int max_width,
    int max_height) :
    width(width),
    height(height)
{
    Utils::assertSingleton(typeid(*this));
    impl_f.frame = this;
    impl_f.layout = new Layout(width, height, max_width, max_height);
    setLayout(impl_f.layout);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::PreventContextMenu);
}

CellWidget* Frame::widgetOfEvent(QMouseEvent* e)
{
    return dynamic_cast<CellWidget*>(childAt(e->pos()));
}

void Frame::reset(void)
{
    impl_f.reset(width, height);
}

void FrameImpl::reset(int width, int height)
{
    layout->reset();
    cell_pressed = nullptr;
    hovered = nullptr;
    under_mouse = nullptr;
    neighbors_pressed.clear();
    indices.clear();
    widgets.resize(width);
    for (auto& column: widgets)
    {
        column.resize(height);
    }
    neighbors.resize(width);
    for (auto& column: neighbors)
    {
        column.resize(height);
        for (auto& neighbors_vec: column)
        {
            neighbors_vec.clear();
        }
    }
    key_reveal_pressed = false;
    // TODO faster reset: use pool for Cell instances
}

void Frame::addCell(int row, int column)
{
    auto widget = dynamic_cast<CellWidget*>(impl_f.layout->itemAtPosition(row, column)->widget());
    widget->reset(FrameImpl::color(column, row, width, height));
    widget->setVisible(true);
    impl_f.indices[widget] = { column, row };
    impl_f.widgets[column][row] = widget;

    // compute neighbors
    auto& n = impl_f.neighbors[column][row];
    for (int x = column - 1; x <= column + 1; ++x)
    {
        for (int y = row - 1; y <= row + 1; ++y)
        {
            if (x >= 0 && y >= 0 && x < width && y < height)
            {
                n.emplace_back(x, y);
            }
        }
    }
    Assert(n.size() <= 9);
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
    impl_f.onCellPressed(nullptr);
    impl_f.key_reveal_pressed = false;
    impl_f.under_mouse = nullptr;
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
        impl_f.pressEvent(impl_f.under_mouse, Qt::LeftButton);
    }
}

void Frame::keyReleaseEvent(QKeyEvent *event)
{
    QWidget::keyReleaseEvent(event);
    if (event->isAutoRepeat()) { return; }
    auto releasing_reveal = event->key() == impl_f.key_reveal;
    auto reveal_was_pressed = impl_f.key_reveal_pressed;
    impl_f.key_reveal_pressed &= !releasing_reveal;
    impl_f.releaseEvent(impl_f.under_mouse,
                 (reveal_was_pressed && releasing_reveal)
                 ? Qt::LeftButton
                 : (event->key() == impl_f.key_flag)
                 ? Qt::RightButton
                 : Qt::NoButton);
}

void Frame::mousePressEvent(QMouseEvent *e)
{
    impl_f.pressEvent(widgetOfEvent(e), e->button());
    QWidget::mousePressEvent(e);
}

void FrameImpl::pressEvent(CellWidget* w, int button)
{
    if (button == Qt::LeftButton)
    {
        onCellPressed(w);
        if (w)
        {
            w->onPress();
        }
    }
}

void Frame::mouseMoveEvent(QMouseEvent *e)
{
    auto* w = widgetOfEvent(e);
    impl_f.under_mouse = w;
    auto pressing_reveal =
        (e->buttons() & Qt::LeftButton) ||
        impl_f.key_reveal_pressed;

    if (pressing_reveal)
    {
        impl_f.pressEvent(w, Qt::LeftButton);
    } else {
        impl_f.hover(w);
    }
}

void Frame::mouseReleaseEvent(QMouseEvent *e)
{
    impl_f.releaseEvent(widgetOfEvent(e), e->button());
    QWidget::mouseReleaseEvent(e);
}

void FrameImpl::releaseEvent(CellWidget* w, int button)
{
    raiseAutoNeighbors();

    if (w && (button == Qt::LeftButton))
    {
        if (w->revealed)
        {
            emit frame->autoRevealNeighbors(indices[w]);
        } else if (w->flag == false)
        {
            emit frame->reveal(indices[w]);
        }
    } else if (w && (button == Qt::RightButton))
    {
        if (w->revealed == false)
        {
            w->switchFlag();
            emit frame->setFlag(indices[w], w->flag);
        }
    }
    cell_pressed = nullptr;
}

void FrameImpl::onCellPressed(CellWidget* w)
{
    if (w == cell_pressed)
    {
        return;
    }
    if (cell_pressed && cell_pressed->revealed == false)
    {
        cell_pressed->raise(true);
    }
    cell_pressed = w;

    // auto reveal : raise old neighbors
    raiseAutoNeighbors();
    // auto reveal : push new neighbors
    if (w != nullptr && w->revealed == true)
    {
        auto w_indices = indices[w];
        auto wx = w_indices.x(), wy = w_indices.y();
        assert(neighbors[wx][wy].size() <= 9);

        for (auto& neighbor_indices: neighbors[wx][wy])
        {
            auto x = neighbor_indices.x(), y = neighbor_indices.y();
            Assert(y < layout->rowCount());
            Assert(x < layout->columnCount());
            auto* n = dynamic_cast<CellWidget*>(layout->itemAtPosition(y, x)->widget());
            if (n->revealed == false && n->flag == false)
            {
                neighbors_pressed.emplace_back(n);
                n->raise(false);
            }
        }
    }
}

void FrameImpl::hover(CellWidget* w)
{
    if (hovered == w)
    {
        return;
    }
    if (hovered != nullptr)
    {
        hovered->hover(false);
    }
    hovered = w;
    if (w != nullptr)
    {
        w->hover(true);
    }
}
