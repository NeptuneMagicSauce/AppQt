#include "Cell.hpp"

#include <QDebug>

using namespace Minus;

Cell::Cell(const QColor& c) :
    widget(c),
    revealed(widget.revealed),
    flag(widget.flag)
{ }

Cell::~Cell(void)
{
    // qDebug() << Q_FUNC_INFO;
}
