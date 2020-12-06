#include "Application.hpp"

#include <QMainWindow>
#include <QAction>
#include <QDebug>

class ApplicationImpl
{
public:
    void installDebugActions(QWidget* w)
    {
        auto* a = new QAction;
        a->setShortcut(QKeySequence("Ctrl+Alt+F1"));
        a->setShortcutContext(Qt::ApplicationShortcut);
        QObject::connect(a, &QAction::triggered, [] () {
            qDebug() << "dereference nullptr";
            qDebug() << *(QPoint*)nullptr;
        });
        w->addAction(a);
    }
} impl_app;

Application::Application(int argc, char** argv) :
    QApplication(argc, argv)
{
    cb.setSingleShot(false);
    cb.setInterval(100);
    connect(&cb, &QTimer::timeout, [this] () {
        for (auto* w: topLevelWidgets())
        {
            if (dynamic_cast<QMainWindow*>(w))
            {
                impl_app.installDebugActions(w);
                cb.stop();
                break;
            }
        }
    });
    cb.start();
}
