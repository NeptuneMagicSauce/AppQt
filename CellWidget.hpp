#include <vector>
#include <QLabel>

namespace Minus
{
    class Cell: public QLabel
    {
        Q_OBJECT
    public:
        Cell(const QColor&);

        void raise(bool);
        void setNeighbors(std::vector<Cell*>& neighbors);

    signals:
        void reveal(Cell&);

    protected:
        virtual void mousePressEvent(QMouseEvent *e) override;
        virtual void mouseReleaseEvent(QMouseEvent *e) override;
    private:
        const QColor color, sunken_color;
        // bool raised { false };
    public:
        bool mine { false };
        bool revealed { false };
        int neighbor_mines { 0 };
        std::vector<Cell*> neighbors;
    };

};
