#pragma once

#include <QFrame>
#include <QAction>
#include <QString>
#include <QTimer>

namespace Utils
{
    class SettingsPane: public QFrame
    {
    public:
        SettingsPane(QWidget* parent);
        QAction* action(const QString& change_label="");

        // register items
        using Callback = std::function<void(QVariant)>;
        void create(QString label, int value, QPoint range, Callback callback, int step=1);

    private:
        QAction m_action;
        QTimer watch_parent_timer;
        QRect parent_geometry;
    };
};
