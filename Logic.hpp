#pragma once

#include <map>
#include <QObject>

#include "Cell.hpp"

namespace Minus
{

    class Logic : public QObject
    {
        Q_OBJECT
    public:
        Logic(void);
        ~Logic(void);

        void reset(int width, int height);
        void reveal(const Indices&);
        void autoRevealNeighbors(const Indices&);
        void setFlag(const Indices&, bool);
        void setRatio(float ratio);

    signals:
        void setMineData(const CellStates&);
        void setMineRevealed(Indices);

    private:
        // state
        int m_width, m_height;
        float m_ratio;
        Cells cells;
        Cells cells_empty;
        std::map<CellPtr, Cells> neighbors; // includes itself as neighbor
        bool any_reveal = false;

        // functions
        CellPtr& cell(int x, int y);
        CellPtr& cell(const Indices&);
        void reveal(const CellPtr&);
        void firstReveal(const CellPtr& first_cell);
        void setOneRandomCellToMine(void);
        int index(int x, int y) const;
        bool indexValid(int x, int y) const;

    public:
        const int& width;
        const int& height;
        const float& ratio;

    };
};
