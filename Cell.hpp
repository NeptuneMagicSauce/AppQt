#pragma once

#include <vector>
#include "CellWidget.hpp"

namespace Minus
{
    class Cell
    {
    public:
        Cell(const QColor& c);
        ~Cell(void);

        // members
        // TODO do not have widget as member, create and destroy them in Frame
        // needs more decoupling: between Logic and CellWidget !
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
