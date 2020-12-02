#include <vector>
#include <QLabel>


namespace Minus
{
    class CellWidget: public QLabel
    {
        Q_OBJECT
    public:
        CellWidget(const QColor&);
        virtual ~CellWidget(void);

        static void reset(void);

        enum struct Depth : int { Raised, Sunken };
        void raise(Depth);
        void revealLabel(void);
        void setLabel(bool mine, int neighbor_mines);

    signals:
        void reveal(void);
        void autoRevealNeighbors(void);

    protected:
        virtual void mousePressEvent(QMouseEvent *e) override;
        virtual void mouseReleaseEvent(QMouseEvent *e) override;
        virtual void mouseMoveEvent(QMouseEvent *e) override;

    public:
        const bool& revealed;
        const bool& flag;
    private:
        bool m_revealed { false };
        bool m_flag { false };
        QString label;
        const QColor color, sunken_color;
        QColor label_color { Qt::white };

        enum struct Action : int { Reveal, Flag };
        void onPress(void);
        void onRelease(Action);

        friend class CellWidgetImpl;
    };

};
