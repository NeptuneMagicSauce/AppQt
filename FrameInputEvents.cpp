#include "FrameInputEvents.hpp"

#include <QMouseEvent>

#include "Utils.hpp"

using namespace Minus;

FrameInputEvents::FrameInputEvents(const int& width, const int& height) :
    width(width),
    height(height)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::PreventContextMenu);
}

void FrameInputEvents::reset(bool need_reset)
{
    cell_pressed = nullptr;
    hovered = nullptr;
    under_mouse = nullptr;
    key_reveal_pressed = false;
    neighbors_pressed.clear();

    if (need_reset)
    {
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

        for (int column=0; column<width; ++column)
        {
            for (int row=0; row<height; ++row)
            {
                auto widget = itemAt(column, row);
                indices[widget] = { column, row };
                widgets[column][row] = widget;

                // compute neighbors
                auto& n = neighbors[column][row];
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
        }
    }
}

CellWidget* FrameInputEvents::widgetOfEvent(QMouseEvent* e)
{
    return dynamic_cast<CellWidget*>(childAt(e->pos()));
}

// TODO call Base::leaveEvent() for all overrides xor do not declare unused param
void FrameInputEvents::leaveEvent(QEvent* event)
{
    // QWidget::leaveEvent(event);
    onCellPressed(nullptr);
    key_reveal_pressed = false;
    under_mouse = nullptr;
    if (hovered)
    {
        hovered->hover(false);
        hovered = nullptr;
    }
}

void FrameInputEvents::keyPressEvent(QKeyEvent *event)
{
    // QWidget::keyPressEvent(event);
    if (event->isAutoRepeat()) { return; }
    auto pressing_reveal = event->key() == key_reveal;
    key_reveal_pressed = pressing_reveal;
    if (pressing_reveal)
    {
        pressEvent(under_mouse, Qt::LeftButton);
    }
}

void FrameInputEvents::keyReleaseEvent(QKeyEvent *event)
{
    // QWidget::keyReleaseEvent(event);
    if (event->isAutoRepeat()) { return; }
    auto releasing_reveal = event->key() == key_reveal;
    auto reveal_was_pressed = key_reveal_pressed;
    key_reveal_pressed &= !releasing_reveal;
    releaseEvent(under_mouse,
                 (reveal_was_pressed && releasing_reveal)
                 ? Qt::LeftButton
                 : (event->key() == key_flag)
                 ? Qt::RightButton
                 : Qt::NoButton);
}

void FrameInputEvents::mousePressEvent(QMouseEvent *e)
{
    // QWidget::mousePressEvent(e);
    pressEvent(widgetOfEvent(e), e->button());
}

void FrameInputEvents::mouseMoveEvent(QMouseEvent *e)
{
    // QWidget::mouseMoveEvent(e);
    auto* w = widgetOfEvent(e);
    under_mouse = w;
    auto pressing_reveal =
        (e->buttons() & Qt::LeftButton) ||
        key_reveal_pressed;

    if (pressing_reveal)
    {
        pressEvent(w, Qt::LeftButton);
    } else {
        hover(w);
    }
}

void FrameInputEvents::mouseReleaseEvent(QMouseEvent *e)
{
    // QWidget::mouseReleaseEvent(e);
    releaseEvent(widgetOfEvent(e), e->button());
}

void FrameInputEvents::pressEvent(CellWidget* w, int button)
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

void FrameInputEvents::releaseEvent(CellWidget* w, int button)
{
    raiseAutoNeighbors();

    if (w && (button == Qt::LeftButton))
    {
        if (w->revealed)
        {
            emit autoRevealNeighbors(indices[w]);
        } else if (w->flag == false)
        {
            emit reveal(indices[w]);
        }
    } else if (w && (button == Qt::RightButton))
    {
        if (w->revealed == false)
        {
            w->switchFlag();
            emit setFlag(indices[w], w->flag);
        }
    }
    cell_pressed = nullptr;
}

void FrameInputEvents::onCellPressed(CellWidget* w)
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
        Assert(neighbors[wx][wy].size() <= 9);

        for (auto& neighbor_indices: neighbors[wx][wy])
        {
            auto x = neighbor_indices.x(), y = neighbor_indices.y();
            auto n = widgets[x][y];
            if (n->revealed == false && n->flag == false)
            {
                neighbors_pressed.emplace_back(n);
                n->raise(false);
            }
        }
    }
}

void FrameInputEvents::hover(CellWidget* w)
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

void FrameInputEvents::raiseAutoNeighbors(void)
{
    for (auto* n: neighbors_pressed)
    {
        n->raise(true);
    }
    neighbors_pressed.clear();
}
