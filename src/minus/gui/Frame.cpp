#include "Frame.hpp"

#include <cmath>
#include <QResizeEvent>
#include <QDebug>

#include "Utils.hpp"

using namespace Utils;
using namespace Minus;

namespace FrameImpl
{
    QColor base_color = Qt::white;
    QColor gradient_color = Qt::black;
    QColor color(int column, int row, int width, int height)
    {
        constexpr auto max_distance = std::sqrt(2.f);
        auto ratio_x = float(column) / (width - 1);
        auto ratio_y = float(row) / (height - 1);
        auto distance =
            std::sqrt(std::pow(ratio_x, 2.f) +
                      std::pow(ratio_y, 2.f))
            / max_distance;
        return Utils::lerpColor(base_color, gradient_color, distance);
    }

    void updateColor(QColor color)
    {
        base_color = color;
        int h,s,v;
        base_color.getHsv(&h, &s, &v);
        s = 255;
        v = 128;
        gradient_color.setHsv(h, s, v);
    }
}

QColor Frame::color(void) const
{
    return FrameImpl::base_color;
}

void Frame::setColor(QColor color)
{
    FrameImpl::updateColor(color);

    for (int column=0; column<width; ++column)
    {
        for (int row=0; row<height; ++row)
        {
            itemAt(column, row)->changeColor(FrameImpl::color(column, row, width, height));
        }
    }
}

Frame::Frame(
    const int& width,
    const int& height,
    int max_width,
    int max_height) :
    FrameInputEvents(width, height),
    set_visible_operation(this),
    layout(width, height, max_width, max_height)
{
    Utils::assertSingleton(typeid(*this));
    setLayout(&layout);
    setMinimumSize(10, 10);
    FrameImpl::updateColor({ 112, 195, 255 });
}

CellWidget* Frame::itemAt(int x, int y)
{
    return dynamic_cast<CellWidget*>(layout.itemAtPosition(y, x)->widget());
}

void Frame::reset(void)
{
    auto need_reset = width != cached_width || height != cached_height;
// #warning debug remove next line
    // need_reset = true;

    cached_width = width;
    cached_height = height;

    FrameInputEvents::reset(need_reset);

    if (need_reset)
    {
        layout.reset(); // set all Widget visibility to false
        if (set_visible_operation.cancel())
        {
            set_visible_indices.clear();
        }
    }

    for (int column=0; column<width; ++column)
    {
        for (int row=0; row<height; ++row)
        {
            itemAt(column, row)->reset(FrameImpl::color(column, row, width, height));
            if (need_reset)
            {
                set_visible_indices.insert(
                    Utils::randomIndex(set_visible_indices.size()),
                    Indices{ column, row });
            }
        }
    }

    if (need_reset)
    {
        set_visible_operation.start( [this] () {
            for (int i=0; i<5; ++i)
            {
                if (set_visible_indices.isEmpty())
                {
                    return 100;
                }

                auto index = set_visible_indices.takeLast();
                itemAt(index.x(), index.y())->setVisible(true);
            }
            if (set_visible_indices.isEmpty())
            {
                return 100;
            }
            return std::min(99, 100 - (set_visible_indices.count() * 100 / (width * height)));
        });
        // TODO disable input while busy loading
    }
}

void Frame::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (indices.empty())
    {
        return;
    }

    // auto scale font size
    const auto size =
        std::max(4.f,
                 std::min(
                     event->size().width() / width,
                     event->size().height() / height)
                 * 0.4f); // default was 0.26
    for (auto w : indices)
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
            widgets[x][y]->setLabel(cell.mine, cell.neighbor_mines);
        }
    }
}

void Frame::revealCell(Indices index)
{
    widgets[index.x()][index.y()]->reveal();
}
