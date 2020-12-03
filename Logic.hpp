#pragma once

#include <map>
#include <random>
#include <QObject>

#include "Cell.hpp"

namespace Minus
{

    class Logic : public QObject
    {
        Q_OBJECT
    public:
        Logic(int width=16, int height=9);
        ~Logic(void);

        // TODO cell() should be private
        CellPtr& cell(int x, int y);

        void reset(int width, int height);
        void reveal(const Indices&);
        void autoRevealNeighbors(const Indices&);
        void setFlag(const Indices&, bool);

    signals:
        void setMineData(const CellStates&);
        void setMineRevealed(Indices);

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
        CellPtr& cell(const Indices&);
        void reveal(const CellPtr&);
        void firstReveal(const CellPtr& first_cell);
        void setOneRandomCellToMine(void);
        int index(int x, int y) const;
        bool indexValid(int x, int y) const;

    public:
        const int& width;
        const int& height;

    };
};
