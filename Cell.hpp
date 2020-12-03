#pragma once

#include <vector>
#include <memory>
#include <QPoint>

namespace Minus
{
    using Indices = QPoint; // order is X-Y column-row

    class Cell
    {
    public:
        Cell(const Indices& indices);
        ~Cell(void);

        // state
        const Indices indices;
        bool revealed { false };
        bool flag { false };
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
