#pragma once

// #include <QWidget>
// #include <QDockWidget>
#include <QDialog>
#include <QAction>

namespace Minus
{
    class SettingsPane: public QDialog
    // class SettingsPane: public QDockWidget
    // class SettingsPane: public QWidget
    {
        Q_OBJECT
    public:
        SettingsPane(QWidget* parent=nullptr);
        QAction* action(void) const;

    protected:
        virtual void showEvent(QShowEvent *event) override;
        virtual void hideEvent(QHideEvent *event) override;
    };
};
