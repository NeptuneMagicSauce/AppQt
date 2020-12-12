#pragma once

#include <QFrame>
#include <QAction>
#include <QString>
#include <QTimer>

namespace Utils
{
    class SettingsPane: public QFrame
    {
        Q_OBJECT
    public:
        SettingsPane(QWidget* parent);
        QAction* action(const QString& change_label="");

        // register items
        int registerInt(QString label, int value, QPoint range);

    signals:
        // TODO signal is private, external api is "provide a callback taking QVariant"
        // reset signal does not contain dimensions
        // instead, gui callback calls logic.setDimensions() then emit reset();

        void integerChanged(int id, int value);

    protected:
        virtual void focusOutEvent(QFocusEvent *event) override;

    private:
        QAction m_action;
        int next_setting_index = 0;
        QTimer watch_parent_timer;
        QRect parent_geometry;
    };
};
