#include <iostream>
#include <array>
#include <vector>
#include <set>
#include <random>

#include <QDebug>
#include <QtGui>
#include <QLayout>
#include <QFrame>
#include <QMainWindow>
#include <QPushButton>
#include <QToolButton>
#include <QApplication>
// #include <QSizePolicy>

namespace Minus
{
    class Window;

    class Cell: public QToolButton
    {
    public:
        Cell(Window& window, int x, int y) :
            window(window),
            x(x),
            y(y)
        {
            static const auto button_size(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
            setSizePolicy(button_size);
            connect(this, &QAbstractButton::clicked, this, &clickedEvent);
        }
        // ~Cell()
        // {
        //     qDebug() << Q_FUNC_INFO << this;
        // }
        QString description(void) const
        {
            return QString::number(x) + "/" + QString::number(y);
        }
        void setMine(void)
        {
            mine = true;
        }
        bool isMine(void) const { return mine; }
        void setNeighbors(int n)
        {
            neighbors = n;
            // setText(mine ? "X" : QString::number(n));
        }
    private:
        Window& window;
        const int x, y;
        bool mine { false };
        int neighbors { 0 };
        void clickedEvent();
    };

    class Window: public QMainWindow
    {
    public:
        Window(int width=30, int height=16) :
            layout(new QGridLayout)
        {
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            auto* central_widget = new QFrame;
            setCentralWidget(central_widget);
            central_widget->setLayout(layout);

            reset(width, height);
        }
        void clicked(int x, int y)
        {
            // qDebug() << "clicked" << x << y << cell(x, y)->description();
            // button(x, y)->setText("clicked");
        }
    private:
        void reset(int width, int height)
        {
            this->width = width;
            this->height = height;
            const int scale = 30;
            resize(scale * width, scale * height);

            while (layout->count())
            {
                layout->removeItem(layout->itemAt(0));
            }

            const auto size = width * height;
            std::vector<Cell*> cells;
            cells.reserve(size);
            // std::set<Cell*> cells;

            for (int x=0; x<width; ++x)
            {
                for (int y=0; y<height; ++y)
                {
                    auto* cell = new Cell(*this, x, y);
                    layout->addWidget(cell, y, x);
                    cells.emplace_back(cell);
                    // cells.insert(cell);
                }
            }

            std::random_device rd;
            std::mt19937 gen(rd());
            gen.seed(time(0));
            std::uniform_int_distribution<int> distrib;
            const int mines = float(size) * 0.20f;

            for (int mine=0; mine<mines; ++mine)
            {
                // qDebug() << "remaining cells" << cells.size();
                distrib.param(std::uniform_int_distribution<int>::param_type(0, int(cells.size() - 1)));
                const auto mine_index = distrib(gen);
                cells[mine_index]->setMine();
                cells.erase(cells.begin() + mine_index);
            }
            // for (int x=0; x<width; ++x)
            // {
            //     for (int y=0; y<height; ++y)
            //     {
            //         std::cout << (cell(x, y)->isMine() ? "x" : "o");
            //     }
            //     std::cout << std::endl;
            // }

            for (int x=0; x<width; ++x)
            {
                for (int y=0; y<height; ++y)
                {
                    int neighbors { 0 };
                    for (int nx=x-1; nx<=x+1; ++nx)
                    {
                        if (nx < 0 || nx >= width) { continue; }
                        for (int ny=y-1; ny<=y+1; ++ny)
                        {
                            if (ny < 0 || ny >= height) { continue; }
                            neighbors += cell(nx, ny)->isMine();
                        }
                    }
                    cell(x, y)->setNeighbors(neighbors);
                }
            }

        }
        QGridLayout* layout;
        int width, height;
        Cell* cell(int x, int y)
        {
            return dynamic_cast<Cell*>(layout->itemAtPosition(y, x)->widget());
        }
    };

    void Cell::clickedEvent()
    {
        setDown(true);
        window.clicked(x, y);
    }
};


int main(int argc, char **argv)
{
    // std::cout << "__cplusplus " << __cplusplus << std::endl;
    // std::cout << "__VERSION__ " << __VERSION__ << std::endl;

    QApplication app(argc, argv);

    Minus::Window window;
    window.show();

    app.exec();

    return 0;
}
