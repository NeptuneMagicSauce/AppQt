#pragma once

#include <map>
#include <random>

#include "Cell.hpp"

namespace Minus
{

    class Logic
    {
    public:
        Logic(int width=16, int height=9);
        ~Logic(void);

        void reset(int width, int height);
        CellPtr& cell(int x, int y);

    private:
        // state
        int m_width, m_height;
        Cells cells;
        Cells cells_empty;
        std::map<CellPtr, Cells> neighbors; // includes itself as neighbor
        bool any_reveal { false };

        // random-ness
        std::random_device rd;
        std::mt19937 gen { rd() };
        std::uniform_int_distribution<int> distrib;

        // functions
        void reveal(CellPtr& cell);
        void autoRevealNeighbors(CellPtr& cell);
        void firstReveal(CellPtr& first_cell);
        void setOneRandomCellToMine(void);
        int index(int x, int y) const;
        bool indexValid(int x, int y) const;

    public:
        const int& width;
        const int& height;

    };
};
