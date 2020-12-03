#pragma once

#include <vector>
#include "CellWidget.hpp"

namespace Minus
{
    class Cell
    {
    public:
        Cell(const QColor& c) :
            widget(c),
            revealed(widget.revealed),
            flag(widget.flag)
        { }

        // members
        CellWidget widget;

        // state
        const bool& revealed;
        const bool& flag;
        bool mine { false };
        int neighbor_mines { 0 };
    };

    // types
    using CellPtr = std::shared_ptr<Cell>;
    using Cells = std::vector<CellPtr>;
};
