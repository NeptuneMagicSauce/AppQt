#include "Frame.hpp"

#include <vector>
#include <QGridLayout>
#include <QResizeEvent>
#include <QDebug>

namespace Minus
{
    class FrameImpl_F
    {
    public:
        Frame* instance { nullptr };
        std::vector<CellWidget*> widgets;
    } impl_f;

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

    Frame::Frame(const int& width, const int& height) :
        layout(new Layout(width, height)),
        width(width),
        height(height)
    {
        assert(!impl_f.instance);
        impl_f.instance = this;
        setLayout(layout);
    }

    void Frame::reset(void)
    {
        while (layout->count())
        {
            layout->takeAt(0);
        }
        impl_f.widgets.clear();
    }

    void Frame::addCell(CellWidget& widget, int row, int column)
    {
        layout->addWidget(&widget, row, column);
        impl_f.widgets.emplace_back(&widget);
    }

    void Frame::resizeEvent(QResizeEvent *event)
    {
        QWidget::resizeEvent(event);

        if (impl_f.widgets.empty())
        {
            return;
        }

        auto font = impl_f.widgets.front()->font();
        const auto size = std::min(
            event->size().width() / width,
            event->size().height() / height)
            * 0.4f; // default was 0.26

        for (auto* w: impl_f.widgets)
        {
            w->setFontSize(size);
        }
    }
};
