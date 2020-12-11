#include "BackgroundOperation.hpp"

#include "Utils.hpp"

using namespace Utils;

BackgroundOperation::BackgroundOperation(QWidget* parent) :
    parent(parent)
{
    Assert(parent != nullptr);

    QObject::connect(&timer, &QTimer::timeout, [this] () {
        internalCallback();
    });

    dialog.setFrameShape(QFrame::StyledPanel);
    dialog.setAutoFillBackground(true);
    dialog.setLayout(&layout);
    dialog.setParent(parent);

    progress_bar.setParent(&dialog);
    progress_bar.setMinimum(0);
    progress_bar.setMaximum(100);

    layout.addWidget(&progress_bar, 0, 0);
}

void BackgroundOperation::start(Callback callback)
{
    this->callback = callback;
    dialog.raise();
    dialog.show();
    timer.start();
}

void BackgroundOperation::internalCallback(void)
{
    Assert(callback != nullptr);
    auto completed = callback();
    if (completed < 100)
    {
        auto parent_size = parent->geometry().size();
        auto dialog_size = QSize{
            std::min(300, parent_size.width()),
            80 };
        dialog.setGeometry(
            {
                QPoint
                {
                    (parent_size.width() - dialog_size.width()) / 2,
                    (parent_size.height() - dialog_size.height()) / 2,
                },
                QSize{ dialog_size.width(), dialog_size.height() }
            });
        progress_bar.setValue(completed);
    }
    else
    {
        timer.stop();
        dialog.hide();
    }
}
