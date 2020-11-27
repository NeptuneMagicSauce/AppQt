#include <iostream>
// #include <array>
#include <vector>
// #include <set>
#include <random>

#include <QDebug>
#include <QtGui>
#include <QLayout>
#include <QFrame>
#include <QMainWindow>
#include <QPushButton>
#include <QToolButton>
#include <QApplication>

#include "Cell.h"

/* potential names

   Minus
   Ninja Minus
   Rominus
   Minus Is Not Ur Solitaire (s...)

 */

using std::vector;

namespace Minus
{
    class CellToRename:
        public Cell
        // public QToolButton
    {
    public:
        using RevealCallback = std::function<void(CellToRename&)>;

        CellToRename(RevealCallback reveal_callback, int x, int y) :
            revealCallback(reveal_callback),
            x(x),
            y(y)
        {
            static const auto button_size(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
            setSizePolicy(button_size);
        }
        virtual void mouseReleaseEvent(QMouseEvent *e) override
        {
            if (e->button() == Qt::LeftButton)
            {
                revealCallback(*this);
            }
        }
        virtual void mouseMoveEvent(QMouseEvent *e) override
        {
            // override to work around this issue:
            // when holding a cell button with the mouse button
            // and leaving it, it raises it
        }

        // helpers
        QString description(void) const
        {
            return QString::number(x) + "/" + QString::number(y);
        }
        void setNeighbors(vector<CellToRename*>& neighbors)
        {
            this->neighbors.swap(neighbors);
            neighbor_mines = 0;
            for (auto* n: this->neighbors)
            {
                neighbor_mines += n->mine;
            }
        }

        // state
        RevealCallback revealCallback;
        const int x, y;
        bool mine { false };
        bool revealed { false };
        vector<CellToRename*> neighbors;
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
            Cell::gen = &gen;

            reset(width, height);
        }

        void reveal(CellToRename& cell)
        {
            // TODO : does this function belong to CellToRename or to Logic ?

            if (cell.revealed)
            {
                return;
            }
            cell.revealed = true;
            cell.setDown(true);
            if (!cell.mine)
            {
                if (cell.neighbor_mines > 0)
                {
                    cell.setText(QString::number(cell.neighbor_mines));
                }
            }
            else
            {
                cell.setText("*");
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

            auto reveal_callback = [this] (CellToRename& c) { reveal(c); };

            for (int x=0; x<width; ++x)
            {
                for (int y=0; y<height; ++y)
                {
                    auto* cell = new CellToRename(reveal_callback, x, y);
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
                    vector<CellToRename*> neighbors;
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
        CellToRename& cell(int x, int y)
        {
            return cell(index(x, y));
        }
        CellToRename& cell(int index)
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
        vector<CellToRename*> cells;

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
