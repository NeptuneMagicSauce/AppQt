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
        ColorDialog(QColor color, QWidget* parent=nullptr);

    signals:
        void valueChanged(QColor);

    private:
        QLabel feedback;
        QColor color;
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
            QPixmap pix;

            void callCallback(int, int);
            void updatePixmap(void);

            static int hue(int x, int w);
            static int sat(int x, int w);
            static int val(int x, int w);
        };
    };
}
