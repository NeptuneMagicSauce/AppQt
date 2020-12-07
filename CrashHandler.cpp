#include "CrashHandler.hpp"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>

using std::string;

bool CrashHandler::hasAlreadyCrashed(void)
{
    static bool has_crashed = false;
    if (has_crashed)
    {
        return true;
    }
    has_crashed = true;
    return false;
}

void CrashHandler::showDialog(const string& error, const string& stack)
{
    // TODO with bold / color / markdown for easier parsing: same as cgdb
    // TODO all items centered: error, button, stack
    // TODO critical icon same as QMessageBox ?

    QDialog dialog {
        nullptr,
        Qt::WindowTitleHint
        | Qt::WindowSystemMenuHint
        | Qt::WindowCloseButtonHint };
    dialog.setWindowTitle("Crash");
    QVBoxLayout layout_root;
    dialog.setLayout(&layout_root);
    QLabel error_label { error.c_str() };
    layout_root.addWidget(&error_label);
    QPushButton button_quit { "Quit" };
    button_quit.setDefault(true);
    button_quit.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(&button_quit, &QPushButton::released, [&dialog](){
        dialog.accept();
    });
    layout_root.addWidget(&button_quit);
    QLabel stack_label { stack.c_str() };
    QScrollArea stack_area;
    stack_area.setWidget(&stack_label);
    layout_root.addWidget(&stack_area);
    dialog.resize(600, 300);
    dialog.exec();
}
