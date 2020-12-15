#pragma once

#include <QPoint>

namespace Minus
{
    using Indices = QPoint; // order is X-Y column-row
    struct CellState
    {
        bool mine;
        int neighbor_mines;
    };
    using CellStates = std::vector<std::vector<CellState>>;
}
