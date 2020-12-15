#include "Layout.hpp"

#include "Utils.hpp"

using namespace Minus;

Layout::Layout(const int& width, const int& height, int max_width, int max_height) :
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

void Layout::setGeometry(const QRect &r)
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

void Layout::reset(void)
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
