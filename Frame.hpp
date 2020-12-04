#pragma once

#include "CellWidget.hpp"
#include "Cell.hpp"

namespace Minus
{
    class Frame: public QWidget
    {
        Q_OBJECT
    public:
        Frame(const int& width, const int& height);
        void reset(void);
        void addCell(int row, int column);
        void setMineData(const CellStates& data);
        void revealCell(Indices);

        const int& width;
        const int& height;

        static constexpr int InitialCellSize { 40 };
        static constexpr int MinimumCellSize { 20 };

        virtual void resizeEvent(QResizeEvent *event) override;

    signals:
        void reveal(Indices);
        void autoRevealNeighbors(Indices);
        void setFlag(Indices, bool);

    protected:
        virtual void mousePressEvent(QMouseEvent *e) override;
        virtual void mouseReleaseEvent(QMouseEvent *e) override;
        virtual void mouseMoveEvent(QMouseEvent *e) override;

    private:

        CellWidget* widgetOfEvent(QMouseEvent* e);
        void onNewCellPressed(CellWidget* w);
        void hover(CellWidget* w);
    };

};
