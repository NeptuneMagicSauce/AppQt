#pragma once

#include <vector>
#include "CellWidget.hpp"

namespace Minus
{
    using Indices = QPoint; // order is X-Y column-row

    class Cell
    {
    public:
        Cell(const QColor& c, const Indices& indices);
        ~Cell(void);

        // members
        // TODO do not have widget as member, create and destroy them in Frame
        // needs more decoupling: between Logic and CellWidget !
        CellWidget widget;

        // state
        const Indices indices;
        const bool& revealed;
        const bool& flag;
        bool mine { false };
        int neighbor_mines { 0 };
    };

    // types
    using CellPtr = std::shared_ptr<Cell>;
    using Cells = std::vector<CellPtr>;

    struct CellState
    {
        bool mine;
        int neighbor_mines;
    };
    using CellStates = std::vector<std::vector<CellState>>;
};
