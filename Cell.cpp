#include "Cell.hpp"

#include <QDebug>

using namespace Minus;

Cell::Cell(const QColor& c, const Indices& indices) :
    widget(c),
    indices(indices),
    revealed(widget.revealed),
    flag(widget.flag)
{ }

Cell::~Cell(void)
{
    // qDebug() << Q_FUNC_INFO;
}
