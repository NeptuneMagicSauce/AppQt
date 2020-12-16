#pragma once

#include <QFrame>
#include <QAction>
#include <QString>
#include <QTimer>
#include <QGroupBox>
#include <QLabel>
#include <QSlider>

namespace Utils
{
    class SettingsPane: public QFrame
    {
    public:
        SettingsPane(QWidget* parent);
        QAction* action(const QString& change_label="");

        // register items
        using Callback = std::function<void(QVariant)>;
        void integer(QString name, QString suffix, int value, QPoint range, int step, Callback callback);
        void button(QString name, Callback callback);
        void color(QString name, QColor value, Callback callback);

    private:
        QAction m_action;
        QTimer watch_parent_timer;
        QRect parent_geometry;

        struct Widgets
        {
            QGroupBox* widget;
            QLabel* value_label;
        };
        Widgets beginCreate(QString name, QString longest_value, QWidget* dialog, QLayout* layout=nullptr);
        void endCreate(QWidget* widget);
    };
};
