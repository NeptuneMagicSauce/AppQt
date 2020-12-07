#include "CrashHandler.hpp"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>

using std::string;

class CrashHandlerImpl
{
public:
    static QWidget* widgetCentered(QWidget* w)
    {
        auto* layout = new QHBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        auto* base = new QWidget;
        base->setLayout(layout);
        auto* spacer_left = new QWidget;
        spacer_left->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        auto* spacer_right = new QWidget;
        spacer_right->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        layout->addWidget(spacer_left);
        layout->addWidget(w);
        layout->addWidget(spacer_right);
        return base;
    }
};

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

void CrashHandler::showDialog(const string& error, const QStringList& stack)
{
    // TODO with bold / color / markdown for easier parsing: same as cgdb

    QDialog dialog {
        nullptr,
        Qt::WindowTitleHint
        | Qt::WindowSystemMenuHint
        | Qt::WindowCloseButtonHint };
    dialog.setWindowTitle("Crash");
    QVBoxLayout layout_root;
    dialog.setLayout(&layout_root);
    QLabel error_label { error.c_str() };
    layout_root.addWidget(CrashHandlerImpl::widgetCentered(&error_label));
    QPushButton button_quit { "Quit" };
    button_quit.setDefault(true);
    button_quit.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QObject::connect(&button_quit, &QPushButton::released, [&dialog](){
        dialog.accept();
    });
    layout_root.addWidget(CrashHandlerImpl::widgetCentered(&button_quit));
    QLabel stack_label;
    QString stack_text;
    for (const auto& s: stack)
    {
        stack_text += s + "\n";
    }
    stack_label.setText(stack_text);
    QScrollArea stack_area;
    stack_area.setWidget(&stack_label);
    layout_root.addWidget(&stack_area);
    dialog.resize(600, 300);
    dialog.exec();
}
