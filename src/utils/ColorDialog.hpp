#pragma once

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QFrame>
#include <QLabel>

namespace Utils
{
    class ColorDialog : public QFrame
    {
        Q_OBJECT
    public:
        ColorDialog(QWidget* parent=nullptr);

    private:
        QLabel feedback;
        QColor color;
        class HSVDialog : public QFrame
        {
        public:
            enum struct Type { Hue, SatVal };
            using Callback = std::function<void(int, int)>;
            HSVDialog(Type type, Callback callback);
        protected:
            QSize sizeHint() const override;
            void paintEvent(QPaintEvent*) override;
            void resizeEvent(QResizeEvent *) override;
            void mouseMoveEvent(QMouseEvent *) override;
            void mousePressEvent(QMouseEvent *) override;
        private:
            const Type type;
            const Callback callback;
            QPixmap pix;
        };
    };
}
