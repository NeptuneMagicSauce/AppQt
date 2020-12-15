#pragma once

#include <QFrame>
#include <QAction>
#include <QString>
#include <QTimer>
#include <QGroupBox>
#include <QLabel>

namespace Utils
{
    class SettingsPane: public QFrame
    {
    public:
        SettingsPane(QWidget* parent);
        QAction* action(const QString& change_label="");

        // register items
        using Callback = std::function<void(QVariant)>;
        void create(QString name, int value, QPoint range, int step, Callback callback);

    private:
        QAction m_action;
        QTimer watch_parent_timer;
        QRect parent_geometry;

        struct Widgets
        {
            QGroupBox* widget;
            QLayout* sub_layout;
            QLabel* value_label;
        };
        Widgets beginCreate(QString name, QString longest_value);
        void endCreate(QWidget* widget);
    };
};
