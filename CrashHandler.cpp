#include "CrashHandler.hpp"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

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
    // TODO custom message box with scroll area for stack trace
    // with bold / color / markdown for easier parsing: same as cgdb
    // and prompt close
    // have stack trace hidden by default same as QMessageBox ?
    // have critical icon same as QMessageBox ?

    QDialog dialog {
        nullptr,
        Qt::WindowTitleHint
        | Qt::WindowSystemMenuHint
        | Qt::WindowCloseButtonHint };
    dialog.setWindowTitle("Crash");
    QVBoxLayout layout_root;
    dialog.setLayout(&layout_root);
    QLabel short_label { error.c_str() };
    layout_root.addWidget(&short_label);
    QPushButton button_quit { "Quit" };
    button_quit.setDefault(true);
    button_quit.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(&button_quit, &QPushButton::released, [&dialog](){
        dialog.accept();
    });
    layout_root.addWidget(&button_quit);
    QLabel detailed_label { stack.c_str() };
    layout_root.addWidget(&detailed_label);
    dialog.exec();
}
