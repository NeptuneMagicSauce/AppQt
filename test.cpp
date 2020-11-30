#include <iostream>
#include <vector>
#include <map>
// #include <array>
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
#include "CellWidget.hpp"
#include "Labels.hpp"

/* potential names

   Minus
   Ninja Minus
   Rominus
   Minus Is Not Ur Solitaire (s...)

 */

using std::vector;
using std::map;

namespace Minus
{
    class Cell
    {
    public:
        Cell(const QColor& c) :
            widget(c)
        { }

        // members
        CellWidget widget;

        // state
        bool mine { false };
        bool revealed { false };
        int neighbor_mines { 0 };
    };


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

            reset(width, height);
        }

        void reveal(Cell& cell)
        {
            if (cell.revealed)
            {
                return;
            }

            cell.revealed = true;
            cell.widget.raise(false);
            cell.widget.enable(false);

            if (!cell.mine)
            {
                cell.widget.setText(Minus::Labels::digits[cell.neighbor_mines]);
            }
            else
            {
                cell.widget.setText(Minus::Labels::bomb);
            }

            if (cell.mine == false && cell.neighbor_mines == 0)
            {
                for (auto* n: neighbors[&cell])
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
            for (auto* c: cells)
            {
                delete c;
            }
            cells.clear();

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
                    auto* cell = new Cell(color);
                    cells[index(x, y)] = cell;
                    layout.addWidget(&cell->widget, y, x);
                    QObject::connect(&cell->widget, &CellWidget::reveal, [this, cell] () {
                            reveal(*cell);
                        });
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
                    vector<Cell*> n;
                    int neighbor_mines = 0;
                    for (int nx=x-1; nx<=x+1; ++nx)
                    {
                        for (int ny=y-1; ny<=y+1; ++ny)
                        {
                            if (indexValid(nx, ny) == false)
                            {
                                continue;
                            }
                            auto& neighbor_cell = cell(nx, ny);
                            neighbor_mines += int(neighbor_cell.mine);
                            n.emplace_back(&neighbor_cell);
                        }
                    }
                    auto& c = cell(x, y);
                    neighbors[&c] = n;
                    c.neighbor_mines = neighbor_mines;
                }
            }

            // warm-up, first call is slow
            QThread::create([this] () {
                for (auto* c: cells)
                {
                    c->widget.setText(" ");
                }
            })->start();

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
        map<Cell*, vector<Cell*>> neighbors;

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
