#include "Frame.hpp"

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
    std::map<CellWidget*, Indices> indices;
    vector<vector<CellWidget*>> widgets;
} impl_f;

Frame::Frame(const int& width, const int& height) :
    layout(new Layout(width, height)),
    width(width),
    height(height)
{
    Utils::assertSingleton(typeid(*this));
    setLayout(layout);
    reset();
}

void Frame::reset(void)
{
    while (layout->count())
    {
        layout->takeAt(0);
    }
    impl_f.cell_pressed = nullptr;
    impl_f.indices.clear();
    impl_f.widgets.resize(width);
    for (auto& column: impl_f.widgets)
    {
        column.resize(height);
    }
}

void Frame::addCell(CellWidget& widget, int row, int column)
{
    layout->addWidget(&widget, row, column);
    impl_f.indices[&widget] = { row, column };
    impl_f.widgets[column][row] = &widget;
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
    w->raise(CellWidget::Depth::Sunken);
    w->revealLabel();
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
        if (w->revealed == false)
        {
            w->flag = !w->flag;
            w->setText(w->flag ? Labels::flag : "");
            qDebug() << "flag" << impl_f.indices[w] << w->flag;
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
        impl_f.cell_pressed->raise(CellWidget::Depth::Raised);
    }
    impl_f.cell_pressed = w;
}
