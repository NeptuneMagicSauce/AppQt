#include "Logic.hpp"

#include <iostream>
#include <set>

#include "Utils.hpp"

using std::set;

using namespace Minus;

Logic::Logic(int width, int height) :
    m_width(width),
    m_height(height),
    width(m_width),
    height(m_height)
{
    Utils::assertSingleton(typeid(*this));
    gen.seed(time(0));

    reset(width, height);

    // warm-up, first call is slow
    cells.front()->widget.setText(" ");
}

Logic::~Logic(void)
{
    // keep a dangling copy of Cells so that they are not needlessly destructed
    // on exit
    auto* copy = new Cells;
    copy->swap(cells);
}

void Logic::reveal(CellPtr& cell)
{
    if (cell->revealed)
    {
        return;
    }

    if (any_reveal == false)
    {
        firstReveal(cell);
    }

    cell->widget.raise(CellWidget::Depth::Sunken);
    cell->widget.revealLabel();

    if (cell->mine == false && cell->neighbor_mines == 0)
    {
        for (auto& n: neighbors[cell])
        {
            reveal(n);
        }
    }

}

void Logic::autoRevealNeighbors(CellPtr& cell)
{
    int flagged_neighbors { 0 };
    Cells to_reveal;
    for (auto& n: neighbors[cell])
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
    if (flagged_neighbors == cell->neighbor_mines)
    {
        for (auto& n: to_reveal)
        {
            reveal(n);
        }
    }
}

void Logic::setOneRandomCellToMine(void)
{
    assert(cells_empty.size());
    distrib.param(std::uniform_int_distribution<int>::param_type(0, int(cells_empty.size() - 1)));
    const auto mine_index = distrib(gen);
    cells_empty[mine_index]->mine = true;
    cells_empty.erase(cells_empty.begin() + mine_index);
}

void Logic::reset(int width, int height)
{
    this->m_width = width;
    this->m_height = height;

    // frame.reset();
    cells.clear();
    neighbors.clear();

    const auto size = width * height;

    cells.resize(size);

    constexpr auto max_distance = std::sqrt(2.f);
    static const QColor
        color_min(112, 195, 255),
        color_max(0, 80, 137);

    for (int x=0; x<width; ++x)
    {
        const auto ratio_x = float(x) / (width - 1);
        for (int y=0; y<height; ++y)
        {
            const auto ratio_y = float(y) / (height - 1);
            const auto distance =
                std::sqrt(std::pow(ratio_x, 2.f) +
                          std::pow(ratio_y, 2.f))
                / max_distance;
            const auto color = Utils::lerpColor(color_min, color_max, distance);
            auto cell = std::make_shared<Cell>(color);
            cells[index(x, y)] = cell;
        }
    }


    const int mines_count = float(size) * 0.20f;

    // populate mines
    cells_empty = cells;
    for (int mine=0; mine<mines_count; ++mine)
    {
        setOneRandomCellToMine();
    }

    any_reveal = false;

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

void Logic::firstReveal(CellPtr& first_cell)
{
    any_reveal = true;

    // remove neighbors from eligible cells for new mines (cells_empty)
    // TODO cache computation of this set in function reset()
    // TODO have reset work on background thread
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

    // count neighbor mines
    for (auto& c: cells)
    {
        int neighbor_mines = 0;
        for (auto& n: neighbors[c])
        {
            neighbor_mines += int(n->mine);
        }
        c->neighbor_mines = neighbor_mines;
        c->widget.setLabel(c->mine, c->neighbor_mines);
    }

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
