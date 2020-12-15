#pragma once

#include "FrameInputEvents.hpp"
#include "Layout.hpp"
#include "BackgroundOperation.hpp"

namespace Minus
{
    class Frame: public FrameInputEvents
    {
    public:
        Frame(
            const int& width,
            const int& height,
            int max_width,
            int max_height);

        void reset(void);
        void setMineData(const CellStates& data);
        void revealCell(Indices);

        virtual void resizeEvent(QResizeEvent *event) override;

    protected:
        virtual CellWidget* itemAt(int x, int y) override;

    private:
        // cost of QWidget::setVisible is heavy with many instances
        Utils::BackgroundOperation set_visible_operation;
        QList<Indices> set_visible_indices;
        Layout layout;
        int cached_width, cached_height;
    };

};
