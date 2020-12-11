#pragma once

#include <QProgressBar>
#include <QFrame>
#include <QTimer>
#include <QGridLayout>

namespace Utils
{
    class BackgroundOperation
    {
    public:
        BackgroundOperation(QWidget* parent);
        QWidget* progressWidget(void) { return &dialog; }

        using Callback = std::function<int()>; // return percent complete in 0-100
        void start(Callback callback);
        bool cancel(void); // return 'was active'

    private:
        QWidget* parent;
        QFrame dialog;
        QProgressBar progress_bar;
        QTimer timer;
        QGridLayout layout;
        Callback callback = nullptr;
        void internalCallback(void);
    };
}
