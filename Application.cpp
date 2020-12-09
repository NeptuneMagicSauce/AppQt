#include "Application.hpp"

#include <vector>
#include <cmath>

#include <QDebug>
#include <QDialog>
#include <QMainWindow>
#include <QGridLayout>
#include <QAction>
#include <QPushButton>
#include <QStyleFactory>

#include "CrashHandler.hpp"
#include "Utils.hpp"

class ApplicationImpl
{
public:
    QDialog* debug_window;

    void installDebugActions(QWidget* w)
    {
        auto* a = new QAction;
        a->setShortcut(QKeySequence("Ctrl+Alt+F1"));
        a->setShortcutContext(Qt::ApplicationShortcut);
        QObject::connect(a, &QAction::triggered, [this] () {
            debug_window->show();
        });
        w->addAction(a);
    }
    void installDebugWindow(void)
    {
        debug_window = new QDialog(
            nullptr,
            Qt::WindowTitleHint
            | Qt::WindowSystemMenuHint
            | Qt::WindowCloseButtonHint);
        debug_window->hide();
        debug_window->setWindowTitle("Debug");
        debug_window->setModal(true);
        auto* layout = new QGridLayout;
        debug_window->setLayout(layout);
        auto installButton = [this, layout] (
            const QString& label,
            std::function<void()> cb) {
            static int index = 0;
            const int columns = 2;
            auto* button = new QPushButton;
            button->setText(label);
            layout->addWidget(button, index / columns, index % columns);
            ++index;
            QObject::connect(button, &QAbstractButton::released, [=](){
                qDebug() << "Debug" << label;
                cb();
            });
        };

        installButton("nullptr", []() {
            qDebug() << *(QPoint*)nullptr;
        });
        installButton("Assert(false)", []() {
            Assert(false);
        });
        installButton("assert(false)", []() {
            assert(false);
        });
        installButton("vector.at(invalid)", []() {
            std::vector<int> v;
            v.resize(0);
            qDebug() << v.at(0);
        });
        installButton("integer div by zero", []() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiv-by-zero"
            qDebug() << 1 / 0;
#pragma GCC diagnostic pop
        });
        installButton("inf", []() {
            qDebug() << 1.0f / 0.0f;
        });
        installButton("nan", []() {
            qDebug() << std::sqrt(-1.0);
        });
        installButton("throw empty", []() {
            throw;
        });
        installButton("throw exception", []() {
            throw std::runtime_error("debug throw");
        });
        installButton("terminate", []() {
            std::terminate();
        });
        installButton("break debugger", []() {
            asm volatile ("int3");
        });
    }
} impl_app;

Application::Application(int argc, char** argv) :
    QApplication(argc, argv)
{
    try
    {
        impl_app.installDebugWindow();

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
    } catch (std::exception& e) {
        PanicException(e);
    }
}

bool Application::notify(QObject *receiver, QEvent *event)
{
    try
    {
        return QApplication::notify(receiver, event);
    } catch (std::exception& e) {
        PanicException(e);
    }
    return true;
}
