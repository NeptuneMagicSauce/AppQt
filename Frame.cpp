#include "Frame.hpp"

#include <vector>
#include <map>
#include <QGridLayout>
#include <QResizeEvent>
#include <QDebug>

#include "Utils.hpp"

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
                return QSize { 40 * width, 40 * height };
            }
        const int& width;
        const int& height;
    };
};

using namespace Minus;

class FrameImpl
{
public:
    CellWidget* cell_pressed { nullptr };
    std::map<CellWidget*, QPoint> indices;
} impl_f;

Frame::Frame(const int& width, const int& height) :
    layout(new Layout(width, height)),
    width(width),
    height(height)
{
    Utils::assertSingleton(typeid(*this));
    setLayout(layout);
}

void Frame::reset(void)
{
    while (layout->count())
    {
        layout->takeAt(0);
    }
    impl_f.cell_pressed = nullptr;
    impl_f.indices.clear();
}

void Frame::addCell(CellWidget& widget, int row, int column)
{
    layout->addWidget(&widget, row, column);
    impl_f.indices[&widget] = { row, column };
}

void Frame::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (impl_f.indices.empty())
    {
        return;
    }

    const auto size = std::min(
        event->size().width() / width,
        event->size().height() / height)
        * 0.4f; // default was 0.26

    for (auto& i: impl_f.indices)
    {
        i.first->setFontSize(size);
    }
}

void Frame::mousePressEvent(QMouseEvent *e)
{
    auto* w = widgetOfEvent(e);
    onNewCellPressed(w);
    if (w && e->button() == Qt::LeftButton)
    {
        w->onPress();
    }
}

void Frame::mouseMoveEvent(QMouseEvent *e)
{
    auto* w = widgetOfEvent(e);
    onNewCellPressed(w);
    if (w && (e->buttons() & Qt::LeftButton))
    {
        w->onPress();
    }
}

void Frame::mouseReleaseEvent(QMouseEvent *e)
{
    auto* w = widgetOfEvent(e);
    if (w && (e->button() == Qt::LeftButton))
    {
        if (w->revealed)
        {
            emit autoRevealNeighbors(impl_f.indices[w]);
        } else if (w->flag == false)
        {
            emit reveal(impl_f.indices[w]);
        }
    } else if (w && (e->button() == Qt::RightButton))
    {
        w->switchFlag();
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
        impl_f.cell_pressed->raise(CellWidget::Depth::Raised);
    }
    impl_f.cell_pressed = w;
}