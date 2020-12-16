#pragma once

#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QFrame>

namespace Utils
{
    class ColorDialog : public QWidget
    {
        Q_OBJECT
    public:
        ColorDialog(QColor color, QWidget* parent=nullptr);

    signals:
        void valueChanged(QColor);

    private:
        QColor color;
    };
}
