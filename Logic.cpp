#include "Logic.hpp"

#include <iostream>
#include <set>

#include <QTime>
#include <QDebug>

#include "Utils.hpp"

using std::set;

using namespace Minus;

Logic::Logic(void) :
    m_width(20),
    m_height(15),
    m_ratio(0.20f),
    width(m_width),
    height(m_height),
    ratio(m_ratio)
{
    Utils::assertSingleton(typeid(*this));

    reset();
}

Logic::~Logic(void)
{
    // keep a dangling copy of Cells so that they are not needlessly destructed
    // on exit
    auto* copy = new Cells;
    copy->swap(cells);
}

void Logic::changeRatio(float ratio)
{
    this->m_ratio = ratio;
}

void Logic::reveal(const Indices& indices)
{
    reveal(cell(indices));
}

void Logic::reveal(const CellPtr& cell)
{
    if (cell->revealed)
    {
        return;
    }

    cell->revealed = true;

    if (any_reveal == false)
    {
        firstReveal(cell);
    }

    emit setMineRevealed(cell->indices);

    if (cell->mine == false && cell->neighbor_mines == 0)
    {
        for (auto& n: neighbors[cell])
        {
            reveal(n);
        }
    }

}

void Logic::autoRevealNeighbors(const Indices& indices)
{
    const auto& c = cell(indices);
    int flagged_neighbors = 0;
    Cells to_reveal;
    for (auto& n: neighbors[c])
    {
        if (n->revealed)
        {
            continue;
        }
        if (n->flag)
        {
            ++flagged_neighbors;
        } else {
            to_reveal.emplace_back(n);
        }
    }
    if (flagged_neighbors == c->neighbor_mines)
    {
        for (auto& n: to_reveal)
        {
            reveal(n);
        }
    }
}

void Logic::setFlag(const Indices& indices, bool flag)
{
    cell(indices)->flag = flag;
}

void Logic::setOneRandomCellToMine(void)
{
    Assert(cells_empty.size());
    const auto mine_index = Utils::randomIndex(cells_empty.size());
    cells_empty[mine_index]->mine = true;
    cells_empty.erase(cells_empty.begin() + mine_index);
}

void Logic::changeWidth(int width)
{
    AssertX(width > 1, "width <= 1 is not supported");
    this->m_width = width;
}

void Logic::changeHeight(int height)
{
    AssertX(height > 1, "height <= 1 is not supported");
    this->m_height = height;
}

void Logic::reset(void)
{
    cells.clear();
    neighbors.clear();
    any_reveal = false;

    const auto size = width * height;
    cells.resize(size);

    for (int x=0; x<width; ++x)
    {
        for (int y=0; y<height; ++y)
        {
            cells[index(x, y)] = std::make_shared<Cell>(Indices(x, y));
        }
    }

    // populate mines
    const int mines_count = float(size) * ratio;
    cells_empty = cells;
    for (int mine=0; mine<mines_count; ++mine)
    {
        setOneRandomCellToMine();
    }

    // populate neighbors
    for (int x=0; x<width; ++x)
    {
        for (int y=0; y<height; ++y)
        {
            Cells cell_neighbors;
            for (int nx=x-1; nx<=x+1; ++nx)
            {
                for (int ny=y-1; ny<=y+1; ++ny)
                {
                    if (indexValid(nx, ny) == false)
                    {
                        continue;
                    }
                    auto& neighbor_cell = cell(nx, ny);
                    cell_neighbors.emplace_back(neighbor_cell);
                }
            }
            auto& c = cell(x, y);
            neighbors[c].swap(cell_neighbors);
        }
    }
}

void Logic::firstReveal(const CellPtr& first_cell)
{
    any_reveal = true;

    // remove neighbors from eligible cells for new mines (cells_empty)
    set<CellPtr> first_cell_neighbors;
    for (auto& n: neighbors[first_cell])
    {
        first_cell_neighbors.insert(n);
    }
    Cells cells_empty_not_neighbors;
    for (auto& c: cells_empty)
    {
        if (first_cell_neighbors.count(c) == 0)
        {
            // not in first cell neighborhood
            cells_empty_not_neighbors.emplace_back(c);
        }
    }
    cells_empty.swap(cells_empty_not_neighbors);

    // move to random empty
    // not in my neighborhood
    for (auto& n: neighbors[first_cell])
    {
        if (n->mine)
        {
            setOneRandomCellToMine();
            n->mine = false;
        }
    }
    cells_empty.clear();

    // signal mine data = is mine and neighbor count
    static CellStates mine_data;
    mine_data.resize(width);
    for (auto& m : mine_data) { m.resize(height); }

    for (int x=0; x<width; ++x)
    {
        for (int y=0; y<height; ++y)
        {
            auto idx = index(x, y);
            auto& c = cells[idx];
            int neighbor_mines = 0;
            for (auto& n: neighbors[c])
            {
                neighbor_mines += int(n->mine);
            }
            c->neighbor_mines = neighbor_mines;
            mine_data[x][y] = { c->mine, c->neighbor_mines };
        }
    }
    emit setMineData(mine_data);

    // print mines and neighbors
    for (int y=0; y<height; ++y)
    {
        for (int x=0; x<width; ++x)
        {
            const auto& c = cell(x, y);
            std::cout << (c->mine ? "x" : std::to_string(c->neighbor_mines));
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

//             // reveal all
//             for (auto& c: cells)
//             {
// #warning debug to remove
//                 reveal(c);
//             }
}

// helper accessors
int Logic::index(int x, int y) const
{
    return y * width + x;
}

bool Logic::indexValid(int x, int y) const
{
    return x >= 0 && y >= 0 && x < width && y < height;
}

CellPtr& Logic::cell(int x, int y)
{
    return cells[index(x, y)];
}

CellPtr& Logic::cell(const Indices& indices)
{
    return cell(indices.x(), indices.y());
}
