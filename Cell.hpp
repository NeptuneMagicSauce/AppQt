#include <vector>
#include <QLabel>

namespace Minus
{
    class Cell: public QLabel
    {
    public:
        using RevealCallback = std::function<void(Cell&)>;
        Cell(const QColor&);
        virtual void mousePressEvent(QMouseEvent *e) override;
        virtual void mouseReleaseEvent(QMouseEvent *e) override;

        void raise(bool);
        void setNeighbors(std::vector<Cell*>& neighbors);

        static void setRevealCallback(RevealCallback);

    private:
        const QColor color, sunken_color;
        bool raised { false };
    public:
        bool mine { false };
        bool revealed { false };
        std::vector<Cell*> neighbors;
        int neighbor_mines { 0 };
    };

};
