#include "Cell.hpp"

#include <QDebug>

using namespace Minus;

// TODO move implementations to header

Cell::Cell(const Indices& indices) :
    indices(indices)
{ }

Cell::~Cell(void)
{
    // qDebug() << Q_FUNC_INFO;
}
