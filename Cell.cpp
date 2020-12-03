#include "Cell.hpp"

#include <QDebug>

using namespace Minus;

// TODO move implementations to header

Cell::Cell(const QColor& c, const Indices& indices) :
    widget(c),
    indices(indices)
{ }

Cell::~Cell(void)
{
    // qDebug() << Q_FUNC_INFO;
}
