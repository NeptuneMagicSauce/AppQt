#include "ColorDialog.hpp"

#include <QPainter>
#include <QDebug>
#include <QHBoxLayout>

using namespace Utils;

class HSVDialog : public QFrame
{
public:
    enum struct Type { Hue, SatVal };
    using Callback = std::function<void(int, int)>;
    HSVDialog(Type type, const QColor& color, HSVDialog* linked_dialog, Callback callback);
protected:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
private:
    const Type type;
    const Callback callback;
    const QColor& color;
    HSVDialog* linked_dialog;
    int pos;
    QPixmap pix;

    void callCallback(int, int);
    void updatePixmap(void);

    static int hue(int x, int w);
    static int sat(int x, int w);
    static int val(int x, int w);

    int position(int hue) const;
    int position(int sat, int val) const;
};

HSVDialog::HSVDialog(
    Type type,
    const QColor& color,
    HSVDialog* linked_dialog,
    Callback callback) :
    type(type),
    callback(callback),
    color(color),
    linked_dialog(linked_dialog),
    pos(0)
{
    setAutoFillBackground(false);
    setFrameStyle(QFrame::StyledPanel);

    if (type == Type::Hue)
    {
        callCallback(color.hue(), 0);
    }
    // no need to call callback for SatVal here because it is linked to Hue
}

void HSVDialog::callCallback(int v0, int v1)
{
    callback(v0, v1);
    if (linked_dialog != nullptr)
    {
        linked_dialog->updatePixmap();
        linked_dialog->update();
    }
}

QSize HSVDialog::sizeHint() const
{
    return { 200, 20 };
}

void HSVDialog::paintEvent(QPaintEvent* )
{
    auto p = QPainter(this);
    drawFrame(&p);
    auto r = contentsRect();
    p.drawPixmap(r.topLeft(), pix);

    auto pt = r.topLeft() + QPoint{ pos, 0 };
    p.setPen(Qt::black);
    p.fillRect(pt.x(), pt.y(), 2, r.height(), Qt::black);
}


int HSVDialog::hue(int x, int w)
{
    return x * 360 / w;
}

int HSVDialog::sat(int x, int w)
{
    return std::min(255, x * 2 * 255 / w);
}

int HSVDialog::val(int x, int w)
{
    return 255 - std::max(0, (x - (w/2)) * 255 / (w/2));
}

int HSVDialog::position(int hue) const
{
    int w = contentsRect().width();
    return hue * w / 360;
}

int HSVDialog::position(int sat, int val) const
{
    int half_w = contentsRect().width() / 2;
    if (val == 255)
    {
        return sat * half_w / 255;
    }
    else
    {
        return 2 * half_w - (val * half_w / 255);
    }
    return 0;
}

void HSVDialog::mousePressEvent(QMouseEvent* m)
{
    if ((m->buttons() & Qt::LeftButton) == Qt::NoButton)
    {
        return;
    }
    auto rect = contentsRect();
    auto x = m->x() - rect.x();
    auto w = rect.width();
    x = std::clamp(x, 0, w - 1);
    if (type == Type::Hue)
    {
        callCallback(x * 360 / w, 0);
    } else if (type == Type::SatVal)
    {
        callCallback(
            std::min(255, x * 2 * 255 / w),
            255 - std::max(0, (x - (w/2)) * 255 / (w/2)));
    }
    pos = x;
    update();
}

void HSVDialog::mouseMoveEvent(QMouseEvent* m)
{
    mousePressEvent(m);
}


void HSVDialog::resizeEvent(QResizeEvent* ev)
{
    QFrame::resizeEvent(ev);
    updatePixmap();
}

void HSVDialog::updatePixmap(void)
{
    int w = width() - frameWidth() * 2;
    int h = height() - frameWidth() * 2;
    auto img = QImage(w, h, QImage::Format_RGB32);
    auto pixel = (uint *) img.scanLine(0);
    QColor c;
    QVector<QColor> colors;
    colors.resize(w);
    for (int x=0; x<w; ++x)
    {
        int h = color.hue();
        int s = 255;
        int v = 255;
        if (type == Type::Hue)
        {
            h = hue(x, w);
        }
        else if (type == Type::SatVal)
        {
            s = sat(x, w);
            v = val(x, w);
        }
        c.setHsv(h, s, v);
        colors[x] = c;
    }
    int x, y;
    for (y = 0; y < h; y++) {
        const uint *end = pixel + w;
        x = 0;
        while (pixel < end) {
            *pixel = colors[x].rgb();
            ++pixel;
            ++x;
        }
    }
    pix = QPixmap::fromImage(img);

    if (type == Type::Hue)
    {
        pos = position(color.hue());
    }
    else if (type == Type::SatVal)
    {
        pos = position(color.saturation(), color.value());
    }
}

ColorDialog::ColorDialog(QColor c, QWidget* parent) :
    QWidget(parent),
    color(c)
{
    setLayout(new QHBoxLayout);
    layout()->setContentsMargins(0, 0, 0, 0);
    auto feedback = new QFrame;
    layout()->addWidget(feedback);
    feedback->setFixedWidth(30);
    feedback->setFixedHeight(50);
    feedback->setAutoFillBackground(false);
    feedback->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

    auto setColor = [this, feedback] (int h, int s, int v) {
        color.setHsv(h, s, v);
        feedback->setStyleSheet("background-color:" + color.name(QColor::HexRgb));
        emit valueChanged(color);
    };

    auto dialogs = new QWidget;
    layout()->addWidget(dialogs);
    dialogs->setLayout(new QVBoxLayout);
    dialogs->layout()->setContentsMargins(0, 0, 0, 0);
    auto widget_satval = new HSVDialog(
        HSVDialog::Type::SatVal, color, nullptr, [this, setColor] (int sat, int val) {
            setColor(color.hue(), sat, val);
        });
    auto widget_hue = new HSVDialog(
        HSVDialog::Type::Hue, color, widget_satval, [this, setColor] (int hue, int) {
            setColor(hue, color.saturation(), color.value());
        });
    dialogs->layout()->addWidget(widget_hue);
    dialogs->layout()->addWidget(widget_satval);

}
