#include "Application.hpp"

#include <vector>
#include <cmath>

#include <QDialog>
#include <QMainWindow>
#include <QGridLayout>
#include <QAction>
#include <QPushButton>
#include <QDebug>

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
        auto* layout = new QGridLayout;
        debug_window->setLayout(layout);
        auto installButton = [this, layout] (
            const QString& label,
            std::function<void()> cb) {
            static int index = 0;
            const int columns = 3;
            auto* button = new QPushButton;
            button->setText(label);
            layout->addWidget(button, index / columns, index % columns);
            ++index;
            QObject::connect(button, &QAbstractButton::released, [=](){
                qDebug() << "Debug" << label;
                cb();
            });
        };

        // TODO catch all these errors in CrashHandler
        // TODO catch exceptions at top level, they are not caught by CrashHandler
        // allow step reverse to error throw
        // TODO port my assert with line function info
// #define ASSERT(check, message) Assert::Assert((check), (#check), (message), __FILE__, __LINE__, __PRETTY_FUNCTION__)
        // for it to be fatal ->
        // needs either crash dialog be modal
        // or CrashDialog quits appli
        // or throw exception or break debugger: only if arch supports catching these

        installButton("nullptr", []() {
            qDebug() << *(QPoint*)nullptr;
        });
        installButton("assert(false)", []() {
            assert(false);
        });
        installButton("vector.at()", []() {
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
        installButton("float div by zero", []() {
            qDebug() << 1.0f / 0.0f;
        });
        installButton("sqrt(-1)", []() {
            qDebug() << std::sqrt(-1.0);
        });
        installButton("throw", []() {
            throw std::runtime_error("debug throw");
        });
        installButton("break debugger", []() {
            asm volatile ("int3");
        });
    }
} impl_app;

Application::Application(int argc, char** argv) :
    QApplication(argc, argv)
{
    impl_app.installDebugWindow();
    // TODO install crash handler early: static ctor, test with crash in member init
    CrashHandler::install();

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
