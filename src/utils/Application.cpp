#include "Application.hpp"

#include <vector>
#include <map>
#include <cmath>
#include <csignal>

#include <QDebug>
#include <QDialog>
#include <QMainWindow>
#include <QGridLayout>
#include <QAction>
#include <QPushButton>
#include <QStyleFactory>

#include "Debugger.hpp"
#include "Utils.hpp"
#include "StackTrace.hpp"

using namespace Utils;

class ApplicationImpl
{
public:
    QDialog* debug_window = nullptr;

    void installDebugActions(QWidget* w)
    {
        auto* a = new QAction;
        a->setShortcut(QKeySequence(
                           // "Ctrl+Alt+F1")); // caught by window manager on linux
                           "Alt+F1"));         // works on windows and linux
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
        auto installButton = [layout] (
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

        installButton("nullptr", []() {
            QPoint* nullPoint = nullptr;
            qDebug() << *nullPoint;
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
        auto sigs = std::map<int, QString> {
            { SIGSEGV , "SEGV" },
            { SIGABRT, "ABORT" },
            { SIGFPE , "FPE" },
            { SIGILL , "ILL" },
            { SIGINT , "INT" },
            { SIGTERM , "TERM" },
        };

        for (auto& i : sigs)
        {
            installButton("signal " + i.second, [i] () {
                std::raise(i.first);
            });
        }

        installButton("asm int3", []() {
            asm volatile ("int3");
        });
        installButton("break debugger", []() {
            Debugger::breakDebugger();
        });
        installButton("print stack trace", []() {
            for (auto& s: StackTrace::getCurrent())
            {
                qDebug() << s.address << s.function << s.location;
            }
        });
    }

    static int& ArgC(int argc)
    {
        // returning a reference for argc passed to QApplication
        // otherwise it crashes on some platforms (linux)
        static int argc_ret = argc;
        return argc_ret;
    }

    // static void* high_dpi_support;
};

// void* ApplicationImpl::high_dpi_support = (
//     // needs to be called before construction of QApplication
// warning: ‘Qt::AA_EnableHighDpiScaling’ is deprecated: High-DPI scaling is always enabled. This attribute no longer has any effect. [-Wdeprecated-declarations]
//   135 |     QApplication::setAttribute(Qt::AA_EnableHighDpiScaling),
//       |                                    ^~~~~~~~~~~~~~~~~~~~~~~
//     QApplication::setAttribute(Qt::AA_EnableHighDpiScaling),
//     nullptr );

Application::Application(int argc, char** argv) :
    QApplication(ApplicationImpl::ArgC(argc), argv)
{
    auto impl = new ApplicationImpl;
    impl->installDebugWindow();

    cb.setSingleShot(false);
    cb.setInterval(100);
    connect(&cb, &QTimer::timeout,
            [this, impl] () {
                for (auto* w: topLevelWidgets())
                {
                    if (dynamic_cast<QMainWindow*>(w))
                    {
                        impl->installDebugActions(w);
                        cb.stop();
                        break;
                    }
                }
            });
    cb.start();
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
