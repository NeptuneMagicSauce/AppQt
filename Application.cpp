#include "Application.hpp"

#include <vector>
#include <cmath>

#include <QMessageBox>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QAction>
#include <QPushButton>
#include <QDebug>

#include "CrashHandler.hpp"

class ApplicationImpl
{
public:
    QMessageBox* debug_window;
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
        debug_window = new QMessageBox;
        debug_window->hide();
        debug_window->setWindowTitle("Debug");
        debug_window->setStandardButtons(QMessageBox::Close);
        debug_window->setEscapeButton(QMessageBox::Close);//nullptr);
        auto install_button = [this] (
            const QString& label,
            std::function<void()> cb) {
            auto* button = new QPushButton;
            button->setText(label);
            // layout->addWidget(button);
            debug_window->addButton(button, QMessageBox::DestructiveRole);
            QObject::connect(button, &QAbstractButton::released, [=](){
                qDebug() << "Debug" << label;
                cb();
            });
        };

        install_button("nullptr", []() {
            qDebug() << *(QPoint*)nullptr;
        });
        install_button("assert(false)", []() {
            assert(false);
        });
        install_button("vector.at()", []() {
            std::vector<int> v;
            v.resize(0);
            qDebug() << v.at(0);
        });
        install_button("integer div by zero", []() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiv-by-zero"
            qDebug() << 1 / 0;
#pragma GCC diagnostic pop
        });
        install_button("float div by zero", []() {
            qDebug() << 1.0f / 0.0f;
        });
        install_button("sqrt(-1)", []() {
            qDebug() << std::sqrt(-1.0);
        });
    }
} impl_app;

Application::Application(int argc, char** argv) :
    QApplication(argc, argv)
{
    impl_app.installDebugWindow();
    CrashHandler::attach();

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
