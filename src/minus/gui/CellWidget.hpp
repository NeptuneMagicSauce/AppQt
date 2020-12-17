#pragma once

#include <QFrame>
#include <QLabel>

class CellWidgetImpl;
namespace Minus
{
    class CellWidget: public QFrame
    {
    public:
        CellWidget(const QColor& color=Qt::white);
        virtual ~CellWidget(void);

        void reset(const QColor& color);
        void changeColor(const QColor& color);
        void raise(bool);
        void setLabel(bool mine, int neighbor_mines);
        void setFontSize(int);
        void reveal(void);
        void onPress(void);
        void switchFlag(void);
        void hover(bool);

    protected:
        virtual void resizeEvent(QResizeEvent* e) override;

    public:
        const bool& flag;
        const bool& revealed;
        const bool& pushed;
    private:
        class LabelOutlined: public QWidget
        {
        public:
            struct Value
            {
                QString text;
                QColor color;
                bool outline;
            };
            LabelOutlined(QWidget* parent);
            void setAlignment(Qt::Alignment alignment);
            void setFont(const QFont& font);
            void show(const Value& value);
            void setFontSize(int font_size);
            void reset(void);
        protected:
            bool outline;
            QLabel main;
            QVector<QLabel*> all_children;
            const QVector<QPoint> offsets;
            virtual void resizeEvent(QResizeEvent* e) override;
            virtual void showEvent(QShowEvent*) override;
            void setVisilityChildren(void);
        };

        bool m_flag = false;
        bool m_revealed = false;
        bool m_pushed = false;
        QColor color, sunken_color, hovered_color;
        LabelOutlined label_outlined;
        LabelOutlined::Value label;
        float font_size_digit, font_size_bomb;
        friend class ::CellWidgetImpl;
    };
};
