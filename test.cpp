#include <iostream>
#include <vector>
#include <map>
#include <set>
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
#include <QToolBar>
#include <QStyle>

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
using std::set;

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
        Layout(int& width, int& height) :
            width(width),
            height(height)
        {
            setContentsMargins(0, 0, 0, 0);
            setSpacing(0);
        }
        virtual void setGeometry(const QRect &r) override
        {
            if (isEmpty())
            {
                QGridLayout::setGeometry(r);
                return;
            }

            assert(count() == width * height);

            const auto cell_size = std::min(
                r.width() / width,
                r.height() / height);
            const auto
                start_x = r.width() / 2 - (cell_size * width) / 2,
                start_y = r.height() / 2 - (cell_size * height) / 2;

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
        virtual QSize minimumSize() const override
        {
            return QSize { 40 * width, 40 * height };
        }
        int& width;
        int& height;
    };

    class Frame: public QWidget
    {
    public:
        Frame(vector<Cell*>& cells, int& width, int& height) :
            layout(width, height),
            cells(cells),
            width(width),
            height(height)
        {
            setLayout(&layout);
        }

        virtual void resizeEvent(QResizeEvent *event) override
        {
            QWidget::resizeEvent(event);

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
        // TODO should cells belong to Frame or Logic ?
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
        // TODO remove widget / layout from logic
    public:
        Logic(int width=16, int height=9) :
            frame(cells, width, height)
        {
            main_window.setCentralWidget(&frame);
            main_window.setWindowTitle("Super Minus");
            main_window.show();
            gen.seed(time(0));
            main_window.addToolBar(Qt::TopToolBarArea, &tool_bar);

            tool_bar.setFloatable(false);
            tool_bar.setMovable(false);
            auto* dummy1 = new QWidget;
            dummy1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            auto* dummy2 = new QWidget;
            dummy2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            tool_bar.addWidget(dummy1);
            auto* action_reset = tool_bar.addAction(
                main_window.style()->standardIcon(QStyle::SP_BrowserReload),
                "Reset",
                [this]() {
                    reset(this->width, this->height);
            });
            tool_bar.addWidget(dummy2);
            tool_bar.setToolButtonStyle(Qt::ToolButtonIconOnly);
            action_reset->setShortcut(QKeySequence::Refresh);
            action_reset->setText(action_reset->text() + " (" + action_reset->shortcut().toString() + ")");

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

            if (any_reveal == false)
            {
                firstReveal(cell);
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

        void setOneRandomCellToMine(void)
        {
            distrib.param(std::uniform_int_distribution<int>::param_type(0, int(cells_empty.size() - 1)));
            const auto mine_index = distrib(gen);
            cells_empty[mine_index]->mine = true;
            cells_empty.erase(cells_empty.begin() + mine_index);
        }

        void reset(int width, int height)
        {
            this->width = width;
            this->height = height;

            while (frame.layout.count())
            {
                frame.layout.takeAt(0);
            }
            for (auto* c: cells)
            {
                delete c;
            }
            cells.clear();
            CellWidget::reset();
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
                    vector<Cell*> cell_neighbors;
                    for (int nx=x-1; nx<=x+1; ++nx)
                    {
                        for (int ny=y-1; ny<=y+1; ++ny)
                        {
                            if (indexValid(nx, ny) == false)
                            {
                                continue;
                            }
                            auto& neighbor_cell = cell(nx, ny);
                            cell_neighbors.emplace_back(&neighbor_cell);
                        }
                    }
                    auto& c = cell(x, y);
                    neighbors[&c] = cell_neighbors;
                }
            }

            // emit resize event
            QResizeEvent e { frame.size(), QSize(0, 0) };
            frame.resizeEvent(&e);
        }

        void firstReveal(Cell& first_cell)
        {
            any_reveal = true;

            // remove neighbors from eligible cells for new mines (cells_empty)
            // TODO cache computation of this set in function reset()
            // TODO have reset work on background thread
            set<Cell*> first_cell_neighbors;
            for (auto* n: neighbors[&first_cell])
            {
                first_cell_neighbors.insert(n);
            }
            vector<Cell*> cells_empty_not_neighbors;
            for (auto* c: cells_empty)
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
            for (auto* n: neighbors[&first_cell])
            {
                if (n->mine)
                {
                    setOneRandomCellToMine();
                    n->mine = false;
                }
            }
            cells_empty.clear();

            // count neighbor mines
            for (auto* c: cells)
            {
                int neighbor_mines = 0;
                for (auto* n: neighbors[c])
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
                    std::cout << (c.mine ? "x" : std::to_string(c.neighbor_mines));
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;

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
        QToolBar tool_bar;
        Frame frame;
        int width, height;
        vector<Cell*> cells;
        vector<Cell*> cells_empty;
        map<Cell*, vector<Cell*>> neighbors; // includes itself as neighbor
        bool any_reveal { false };

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
