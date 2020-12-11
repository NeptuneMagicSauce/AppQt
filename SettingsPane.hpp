#pragma once

#include <QFrame>
#include <QAction>
#include <QString>

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
        void integerChanged(int id, int value);

    protected:
        virtual void showEvent(QShowEvent *event) override;

    private:
        QAction m_action;
        int next_setting_index = 0;
    };
};
