#include <vector>
#include <QLabel>

namespace Minus
{
    class CellWidget: public QLabel
    {
        Q_OBJECT
    public:
        enum struct Depth : int { Raised, Sunken };
        CellWidget(const QColor&);
        void enable(bool);
        void raise(Depth);
        void setLabel(bool mine, int neighbor_mines);

    signals:
        void reveal(void);

    protected:
        virtual void mousePressEvent(QMouseEvent *e) override;
        virtual void mouseReleaseEvent(QMouseEvent *e) override;

    private:
        bool enabled { true };
        QString label;
        const QColor color, sunken_color;
        QColor label_color { Qt::white };
    };

};
