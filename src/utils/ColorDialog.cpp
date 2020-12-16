#include "ColorDialog.hpp"

#include <QPainter>
#include <QHBoxLayout>

using namespace Utils;

ColorDialog::HSVDialog::HSVDialog(Type type, QColor color, Callback callback) :
    type(type),
    callback(callback)
{
    setAutoFillBackground(false);
    setFrameStyle(QFrame::StyledPanel);

    if (type == Type::Hue)
    {
        callback(color.hue(), 0);
    }
    else if (type == Type::SatVal)
    {
        callback(color.saturation(), color.value());
    }
}

QSize ColorDialog::HSVDialog::sizeHint() const
{
    return { 200, 20 };
}

void ColorDialog::HSVDialog::paintEvent(QPaintEvent* )
{
    QPainter p(this);
    drawFrame(&p);
    QRect r = contentsRect();
    p.drawPixmap(r.topLeft(), pix);
}

void ColorDialog::HSVDialog::mouseMoveEvent(QMouseEvent *)
{
}

void ColorDialog::HSVDialog::mousePressEvent(QMouseEvent *m)
{
    auto rect = contentsRect();
    int w = m->x() - rect.x();
    if (type == Type::Hue)
    {
        callback(w * 360 / rect.width(), 0);
    } else if (type == Type::SatVal)
    {
        callback(w * 255 / rect.width(), 255);
    }
}


void ColorDialog::HSVDialog::resizeEvent(QResizeEvent *ev)
{
    QFrame::resizeEvent(ev);

    int w = width() - frameWidth() * 2;
    int h = height() - frameWidth() * 2;
    QImage img(w, h, QImage::Format_RGB32);
    int x, y;
    uint *pixel = (uint *) img.scanLine(0);
    // TODO first loop in width
    for (y = 0; y < h; y++) {
        const uint *end = pixel + w;
        x = 0;
        while (pixel < end) {
            QPoint p(x, y);
            QColor c;
            int hue = 360;
            int sat = 255;
            int val = 255;
            if (type == Type::Hue)
            {
                hue = x * 360 / w;
            }
            else if (type == Type::SatVal)
            {
                sat = std::min(255, x * 2 * 255 / w);
                val = 255 - std::max(0, (x - (w/2)) * 255 / (w/2));
            }
            c.setHsv(hue, sat, val);
            *pixel = c.rgb();
            ++pixel;
            ++x;
        }
    }
    pix = QPixmap::fromImage(img);
}

ColorDialog::ColorDialog(QColor c, QWidget* parent) :
    QFrame(parent),
    color(c)
{
    setLayout(new QHBoxLayout);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(&feedback);
    feedback.setFixedWidth(30);
    feedback.setFixedHeight(50);
    feedback.setAutoFillBackground(true); // TODO auto fill ?
    feedback.setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

    auto setColor = [this] (int h, int s, int v) {
        color.setHsv(h, s, v);
        feedback.setStyleSheet("background-color:" + color.name(QColor::HexRgb));
    };

    auto dialogs = new QWidget;
    layout()->addWidget(dialogs);
    dialogs->setLayout(new QVBoxLayout);
    dialogs->layout()->setContentsMargins(0, 0, 0, 0);
    dialogs->layout()->addWidget(
        new HSVDialog(HSVDialog::Type::Hue, color, [this, setColor] (int hue, int) {
            setColor(hue, color.saturation(), color.value());
        }));
    dialogs->layout()->addWidget(
        new HSVDialog(HSVDialog::Type::SatVal, color, [this, setColor] (int sat, int val) {
            setColor(color.hue(), sat, val);
        }));

}
