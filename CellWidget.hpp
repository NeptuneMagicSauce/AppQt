#include <vector>
#include <QLabel>

namespace Minus
{
    class CellWidget: public QLabel
    {
        Q_OBJECT
    public:
        CellWidget(const QColor&);
        void enable(bool);
        void raise(bool);

    signals:
        void reveal(void);

    protected:
        virtual void mousePressEvent(QMouseEvent *e) override;
        virtual void mouseReleaseEvent(QMouseEvent *e) override;

    private:
        bool enabled { true };
        const QColor color, sunken_color;
    };

};
