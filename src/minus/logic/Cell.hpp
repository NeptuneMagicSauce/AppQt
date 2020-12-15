#pragma once

#include <vector>
#include <memory>

#include "CellState.hpp"

namespace Minus
{
    class Cell
    {
    public:
        Cell(const Indices& indices) :
            indices(indices)
            { }

        // state
        const Indices indices;
        bool revealed = false;
        bool flag = false;
        bool mine = false;
        int neighbor_mines = 0;
    };

    // types
    using CellPtr = std::shared_ptr<Cell>;
    using Cells = std::vector<CellPtr>;
};
