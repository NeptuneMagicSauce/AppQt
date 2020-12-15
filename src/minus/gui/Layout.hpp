#pragma once

#include <QGridLayout>
#include "Pool.hpp"
#include "CellWidget.hpp"

namespace Minus
{
    class Layout: public QGridLayout
    {
    public:
        Layout(const int& width, const int& height, int max_width, int max_height);
        virtual void setGeometry(const QRect &r) override;
        void reset(void);
    private:
        const int& width;
        const int& height;
        const int max_width, max_height;
        Utils::Pool<CellWidget> pool;
    };
}
