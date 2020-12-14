#pragma once

#include <vector>
#include <map>

#include "CellWidget.hpp"
#include "CellState.hpp"

namespace Minus
{
    class FrameInputEvents: public QWidget
    {
        Q_OBJECT
    public:
        const int& width;
        const int& height;

    signals:
        void reveal(Indices);
        void autoRevealNeighbors(Indices);
        void setFlag(Indices, bool);
        void anyActivity(void);

    protected:
        FrameInputEvents(
            const int& width,
            const int& height);
        void reset(bool need_reset);

        virtual void mousePressEvent(QMouseEvent *e) override;
        virtual void mouseReleaseEvent(QMouseEvent *e) override;
        virtual void mouseMoveEvent(QMouseEvent *e) override;
        virtual void keyPressEvent(QKeyEvent *event) override;
        virtual void keyReleaseEvent(QKeyEvent *event) override;
        virtual void leaveEvent(QEvent *event) override;

        virtual CellWidget* itemAt(int x, int y) = 0;

    private:
        CellWidget* widgetOfEvent(QMouseEvent* e);
        void reset(void);
        void onCellPressed(CellWidget* w);
        void pressEvent(CellWidget* w, int button);
        void releaseEvent(CellWidget* w, int button);
        void hover(CellWidget* w);
        void raiseAutoNeighbors(void);

        CellWidget* cell_pressed = nullptr;
        CellWidget* hovered = nullptr;
        CellWidget* under_mouse = nullptr;
        std::vector<CellWidget*> neighbors_pressed;

        int key_reveal = Qt::Key_S;
        int key_flag = Qt::Key_D;
        bool key_reveal_pressed = false;

        std::vector<std::vector<std::vector<Indices>>> neighbors;
    protected:
        std::map<CellWidget*, Indices> indices;
        std::vector<std::vector<CellWidget*>> widgets;
    };
}
