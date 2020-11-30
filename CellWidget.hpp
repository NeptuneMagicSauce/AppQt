#include <vector>
#include <QLabel>

namespace Minus
{
    class CellWidget: public QLabel
    {
        Q_OBJECT
    public:
        CellWidget(const QColor&);

        void raise(bool);
        void setNeighbors(std::vector<CellWidget*>& neighbors);

    signals:
        void reveal(CellWidget&);

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
        std::vector<CellWidget*> neighbors;
    };

};
