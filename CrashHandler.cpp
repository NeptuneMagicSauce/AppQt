#include "CrashHandler.hpp"

#include <iostream>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QDebug>

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

void CrashHandler::showTerminal(const std::string& error, const Stack& stack)
{
    std::cerr << error << std::endl;
    auto qdebug = qDebug();
    qdebug.noquote();
    qdebug.nospace();
    for (const auto& s : stack)
    {
        qdebug << s.prettyPrint(false) << "\n";
    }
}

void CrashHandler::showDialog(const string& error, const Stack& stack)
{
    // TODO with bold / color / markdown for easier parsing: same as cgdb
    // also same color and splitting for terminal output

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
        stack_text += s.prettyPrint(true) + "\n";
    }
    stack_label.setText(stack_text);
    QScrollArea stack_area;
    stack_area.setWidget(&stack_label);
    layout_root.addWidget(&stack_area);
    dialog.resize(600, 300);
    dialog.exec();
}

CrashHandler::Stack CrashHandler::formatStack(const QStringList& stack)
{
    auto format_location = [] (const QString& l) {
        auto ret = l;
        ret.replace("c:/Devel/Workspace/", "", Qt::CaseInsensitive);
        ret.replace("c:/Devel/Tools/", "", Qt::CaseInsensitive);
        return ret;
    };

    Stack ret;
    for (auto& s: stack)
    {
        StackInfo info;
        auto split_addr = s.split(": ");
        if (split_addr.size() <= 1)
        {
            info.address = format_location(s.trimmed());
            continue;
        }
        info.address = split_addr[0].trimmed();
        auto split_function_location = split_addr[1].split(" at ");
        if (split_function_location.size() <= 1)
        {
            info.function = format_location(split_addr[1].trimmed());
        } else {
            info.function = split_function_location[0].trimmed();
            info.location = "at " + format_location(split_function_location[1].trimmed());
        }
        ret << info;
    }
    return ret;
}

QString CrashHandler::StackInfo::prettyPrint(bool has_horizontal_scroll) const
{
    auto length = address.size() + function.size() + location.size();
    if (length <= 80)
    {
        return address + ": " + function + " at " + location;
    }

    auto split_length = [] (const QString& s) {
        QStringList ret;
        auto tmp = s;
        while (!tmp.isEmpty())
        {
            ret << tmp.left(80);
            tmp.remove(0, 80);
        }
        return ret;
    };

    auto ret = address + "\n";
    if (has_horizontal_scroll)
    {
        ret +=
            "\t" + function + "\n" +
            "\t" + location;
    } else {
        for (auto& f: split_length(function))
        {
            ret += "\t" + f + "\n";
        }
        for (auto& l: split_length(location))
        {
            ret += "\t" + l + "\n";
        }
    }
    return ret.trimmed();
}
