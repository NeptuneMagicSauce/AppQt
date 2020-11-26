#include <iostream>
#include <array>

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
    class Window: public QMainWindow
    {
    public:
        Window() :
            layout(new QGridLayout)
        {
            // const int xx = 30, yy = 16;
            // const int xx = 16, yy = 16;
            const int xx = 9, yy = 9;

            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);

            const int scale = 30;
            resize(scale * xx, scale * yy);
            auto* central_widget = new QFrame;
            setCentralWidget(central_widget);
            central_widget->setLayout(layout);
            cells.resize(xx);
            for (int x=0; x<xx; ++x)
            {
                cells[x].resize(yy);
                for (int y=0; y<yy; ++y)
                {
                    cells[x][y] = 0;
                    // auto* button = new QPushButton;
                    auto* button = new QToolButton;

                    button->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
                    // button->setText(QString::number(x) + "/" + QString::number(y));
                    layout->addWidget(button, y, x);
                    connect(button, &QPushButton::clicked, [x,y, this] () {
                        clicked(x, y);
                    });
                }
            }
        }
    private:
        QGridLayout* layout;
        void clicked(int x, int y)
        {
            // qDebug() << "clicked" << x << y;
            // button(x, y)->setText("clicked");
        }
        QPushButton* button(int x, int y)
        {
            auto* itemat = layout->itemAtPosition(y, x);
            // qDebug() << "item at ok";
            auto* cast = dynamic_cast<QPushButton*>(itemat->widget());
            // qDebug() << "dynamic cast ok" << cast;
            return cast;
        }
        std::vector<std::vector<int>> cells;
    };
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
