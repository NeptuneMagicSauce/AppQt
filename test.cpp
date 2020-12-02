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

#include "LoadContent.hpp"
#include "Utils.hpp"
#include "CellWidget.hpp"

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

    class Layout: public QGridLayout
    {
    public:
        Layout(vector<Cell*>& cells, int& width, int& height) :
            cells(cells),
            width(width),
            height(height)
        {
            setContentsMargins(0, 0, 0, 0);
            setSpacing(0);
        }
        virtual void setGeometry(const QRect &r) override
        {
            if (cells.empty())
            {
                QGridLayout::setGeometry(r);
                return;
            }

            const auto cell_size = std::min(
                r.width() / width,
                r.height() / height);
            const auto
                start_x = (r.width() - (cell_size * width)) / 2,
                start_y = (r.height() - (cell_size * height)) / 2;

            for (int x=0; x<width; ++x)
            {
                for (int y=0; y<height; ++y)
                {
                    const QRect rect
                        {
                            start_x + cell_size * x,
                            start_y + cell_size * y,
                            cell_size,
                            cell_size,
                        };
                    itemAtPosition(y, x)->setGeometry(rect);
                }
            }
        }
        vector<Cell*>& cells;
        int& width;
        int& height;
    };

    class Frame: public QWidget
    {
    public:
        Frame(vector<Cell*>& cells, int& width, int& height) :
            layout(cells, width, height),
            cells(cells),
            width(width),
            height(height)
        {
            setLayout(&layout);
        }

        virtual void resizeEvent(QResizeEvent *event) override
        {
            QWidget::resizeEvent(event);
            // qDebug() << event->oldSize() << "->" << event->size();

            if (cells.empty())
            {
                return;
            }

            auto font = cells.front()->widget.font();
            const auto size = std::min(
                event->size().width() / width,
                event->size().height() / height)
                * 0.4f; // default was 0.26

            for (auto* c: cells)
            {
                font.setPointSizeF(size * sizes.at(c->mine));
                c->widget.setFont(font);
            }
        }

        Layout layout;
        vector<Cell*>& cells;
        int& width;
        int& height;
        const map<bool, float> sizes
        {
            { true,  0.9f },
            { false, 1.0f },
        };
    };


    class Logic
    {
    public:
        Logic(int width=16, int height=9) :
            frame(cells, width, height)
        {
            main_window.setCentralWidget(&frame);
            main_window.setWindowTitle("Super Minus");
            main_window.show();
            gen.seed(time(0));

            reset(width, height);

            // warm-up, first call is slow
            cells.front()->widget.setText(" ");
        }

    private:
        void reveal(Cell& cell)
        {
            if (cell.revealed)
            {
                return;
            }

            cell.widget.raise(CellWidget::Depth::Sunken);
            cell.widget.revealLabel();

            if (cell.mine == false && cell.neighbor_mines == 0)
            {
                for (auto* n: neighbors[&cell])
                {
                    reveal(*n);
                }
            }

        }

        void autoRevealNeighbors(Cell& cell)
        {
            int flagged_neighbors { 0 };
            vector<Cell*> to_reveal;
            for (auto* n: neighbors[&cell])
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
            if (flagged_neighbors == cell.neighbor_mines)
            {
                for (auto* n: to_reveal)
                {
                    reveal(*n);
                }
            }
        }

        void reset(int width, int height)
        {
            this->width = width;
            this->height = height;
            const int scale = 40;
            main_window.resize(scale * width, scale * height);

            while (frame.layout.count())
            {
                frame.layout.removeItem(frame.layout.itemAt(0));
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
                    frame.layout.addWidget(&cell->widget, y, x);
                    QObject::connect(&cell->widget, &CellWidget::reveal, [this, cell] () {
                            reveal(*cell);
                        });
                    QObject::connect(&cell->widget, &CellWidget::autoRevealNeighbors, [this, cell] () {
                            autoRevealNeighbors(*cell);
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

                    c.widget.setLabel(c.mine, c.neighbor_mines);
                }
            }

            // emit resize event
            QResizeEvent e { frame.size(), QSize(0, 0) };
            frame.resizeEvent(&e);

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

//             // reveal all
//             for (auto* c: cells)
//             {
// #warning debug to remove
//                 reveal(*c);
//             }

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
        Frame frame;
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

    LoadContent::doLoad();

    Minus::Logic logic;

    app.exec();

    return 0;
}
