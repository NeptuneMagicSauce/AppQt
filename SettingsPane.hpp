#pragma once

#include <QDialog>
#include <QAction>
#include <QString>

namespace Utils
{
    class SettingsPane: public QDialog
    {
        Q_OBJECT
    public:
        SettingsPane(QWidget* parent=nullptr);
        QAction* action(const QString& label="");

        // register items
        int registerInt(QString label, int value, QPoint range);

    signals:
        void intChanged(int id, int value);

    protected:
        virtual void showEvent(QShowEvent *event) override;
        virtual void hideEvent(QHideEvent *event) override;
    };
};
