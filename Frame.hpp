#pragma once

#include "CellWidget.hpp"
#include "CellState.hpp"

namespace Minus
{
    class Frame: public QWidget
    {
        Q_OBJECT
    public:
        Frame(
            const int& width,
            const int& height,
            int max_width,
            int max_height);
        void reset(void);
        void setMineData(const CellStates& data);
        void revealCell(Indices);

        const int& width;
        const int& height;

        virtual void resizeEvent(QResizeEvent *event) override;

    signals:
        void reveal(Indices);
        void autoRevealNeighbors(Indices);
        void setFlag(Indices, bool);

    protected:
        virtual void mousePressEvent(QMouseEvent *e) override;
        virtual void mouseReleaseEvent(QMouseEvent *e) override;
        virtual void mouseMoveEvent(QMouseEvent *e) override;
        virtual void keyPressEvent(QKeyEvent *event) override;
        virtual void keyReleaseEvent(QKeyEvent *event) override;
        virtual void leaveEvent(QEvent *event) override;

    private:
        CellWidget* widgetOfEvent(QMouseEvent* e);
    };

};
