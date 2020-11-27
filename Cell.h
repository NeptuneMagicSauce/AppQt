#include <random>
#include <QToolButton>
#include <QFrame>

namespace Minus
{
    class Cell: public QFrame
    {
    public:
        Cell();
        void setDown(bool) { }
        void setText(const QString&) { }

        static std::mt19937* gen;

        virtual void mousePressEvent(QMouseEvent *e) override;
    private:

        static std::uniform_int_distribution<int> distrib;
        const QColor color;
        void raise(bool);
    };

};
