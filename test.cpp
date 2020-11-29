#include <iostream>
// #include <array>
#include <vector>
// #include <set>
#include <random>
#include <cmath>

#include <QDebug>
#include <QtGui>
#include <QLayout>
#include <QFrame>
#include <QMainWindow>
#include <QPushButton>
#include <QToolButton>
#include <QApplication>

#include "Utils.hpp"
#include "Cell.hpp"
#include "Labels.hpp"

/* potential names

   Minus
   Ninja Minus
   Rominus
   Minus Is Not Ur Solitaire (s...)

 */

using std::vector;

namespace Minus
{
    class Logic
    {
    public:
        Logic(int width=30, int height=16)
        {
            layout.setContentsMargins(0, 0, 0, 0);
            layout.setSpacing(0);
            auto* central_widget = new QFrame;
            main_window.setCentralWidget(central_widget);
            central_widget->setLayout(&layout);
            main_window.setWindowTitle("Super Minus");
            main_window.show();
            gen.seed(time(0));
            auto reveal_callback = [this] (Cell& c) { reveal(c); };
            Cell::setRevealCallback(reveal_callback);

            reset(width, height);
        }

        void reveal(Cell& cell)
        {
            // TODO : does this function belong to Cell or to Logic ?
            // TODO : couple reveal and raise, call them in only 1 place Cell or Logic
            // -> have widget for cell only do gui/rendering with signal revealed
            // and slot do/trigger reveal = keep down and emit signal

            // TODO : store instances in only 1 place : vector<Cell> no pointers
            // neighbors can be indices rather than pointers/instances
            // xor store neighbors in Logic not Cell

            // TODO fix emacs and cmake
            // emacs error = can not find make
            // cmake error = can not find compiler
            // enable -Wall -Wextra

            if (cell.revealed)
            {
                return;
            }
            cell.revealed = true;
            cell.raise(false);

            if (!cell.mine)
            {
                if (cell.neighbor_mines > 0)
                {
                    cell.setText(Minus::Labels::digits[cell.neighbor_mines]);
                }
            }
            else
            {
                cell.setText(Minus::Labels::bomb);
            }

            if (cell.mine == false && cell.neighbor_mines == 0)
            {
                for (auto* n: cell.neighbors)
                {
                    reveal(*n);
                }
            }

        }
    private:
        void reset(int width, int height)
        {
            this->width = width;
            this->height = height;
            const int scale = 30;
            main_window.resize(scale * width, scale * height);

            while (layout.count())
            {
                layout.removeItem(layout.itemAt(0));
            }

            const auto size = width * height;
            cells.clear();
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
                    auto* cell = new Cell(
                        // reveal_callback,
                        Utils::lerpColor(color_min, color_max, distance));
                    layout.addWidget(cell, y, x);
                    cells[index(x, y)] = cell;
                }
            }


            const int mines = float(size) * 0.20f;

            // populate mines
            auto cells_temp = cells;
            for (int mine=0; mine<mines; ++mine)
            {
                // qDebug() << "remaining cells" << cells.size();
                distrib.param(std::uniform_int_distribution<int>::param_type(0, int(cells_temp.size() - 1)));
                const auto mine_index = distrib(gen);
                cells_temp[mine_index]->mine = true;
                cells_temp.erase(cells_temp.begin() + mine_index);
            }

            // populate neighbors
            for (int x=0; x<width; ++x)
            {
                for (int y=0; y<height; ++y)
                {
                    vector<Cell*> neighbors;
                    for (int nx=x-1; nx<=x+1; ++nx)
                    {
                        for (int ny=y-1; ny<=y+1; ++ny)
                        {
                            if (indexValid(nx, ny) == false) { continue; }
                            neighbors.emplace_back(&cell(nx, ny));
                        }
                    }
                    cell(x, y).setNeighbors(neighbors);
                }
            }

            // print mines and neighbors
            for (int y=0; y<height; ++y)
            {
                for (int x=0; x<width; ++x)
                {
                    const auto& c = cell(x, y);
                    std::cout << (c.mine ? "x" : std::to_string(c.neighbor_mines));
                }
                std::cout << std::endl;
            }

        }

        // helper accessors
        Cell& cell(int x, int y)
        {
            return cell(index(x, y));
        }
        Cell& cell(int index)
        {
            return *cells[index];
        }
        int index(int x, int y) const
        {
            return y * width + x;
        }
        bool indexValid(int x, int y) const
        {
            return x >= 0 && y >= 0 && x < width && y < height;
        }


        // state
        QMainWindow main_window;
        QGridLayout layout;
        int width, height;
        vector<Cell*> cells;

        // random-ness
        std::random_device rd;
        std::mt19937 gen { rd() };
        std::uniform_int_distribution<int> distrib;

    };
};


int main(int argc, char **argv)
{
    // std::cout << "__cplusplus " << __cplusplus << std::endl;
    // std::cout << "__VERSION__ " << __VERSION__ << std::endl;

    QApplication app(argc, argv);

    Minus::Logic logic;

    app.exec();

    return 0;
}
