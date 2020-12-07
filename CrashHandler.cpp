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

    auto widgetCentered = [] (QWidget* w)
    {
        auto* layout = new QHBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        auto* base = new QWidget;
        base->setLayout(layout);
        auto* spacer_left = new QWidget;
        spacer_left->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        auto* spacer_right = new QWidget;
        spacer_right->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        layout->addWidget(spacer_left);
        layout->addWidget(w);
        layout->addWidget(spacer_right);
        return base;
    };

    QDialog dialog {
        nullptr,
        Qt::WindowTitleHint
        | Qt::WindowSystemMenuHint
        | Qt::WindowMaximizeButtonHint
        | Qt::WindowCloseButtonHint };
    dialog.setWindowTitle("Crash");
    QVBoxLayout layout_root;
    dialog.setLayout(&layout_root);
    QLabel error_label { error.c_str() };
    layout_root.addWidget(widgetCentered(&error_label));
    QPushButton button_quit { "Quit" };
    button_quit.setDefault(true);
    QObject::connect(&button_quit, &QPushButton::released, [&dialog](){
        dialog.accept();
    });
    layout_root.addWidget(widgetCentered(&button_quit));
    QLabel stack_label;
    QString stack_text;
    for (const auto& s: stack)
    {
        stack_text += s.prettyPrint(true) + "\n";
    }
    stack_label.setText(stack_text);
    QScrollArea stack_area;
    stack_area.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    stack_area.setWidget(&stack_label);
    layout_root.addWidget(&stack_area);
    dialog.resize(800, 500);
    dialog.exec();
}

// TODO coding style: lambdas should be camelCase()

CrashHandler::Stack CrashHandler::formatStack(const QStringList& stack)
{
    Stack ret;
    for (auto s: stack)
    {
        StackInfo info;

        auto take_before = [] (QString& s, const QString& pattern) {
            auto pattern_len = pattern.size();
            auto len =
                pattern_len
                ? s.indexOf(pattern)
                : s.size();
            if (len == -1)
            {
                auto ret = s;
                s.resize(0);
                return ret.trimmed();
            }
            auto ret = s.left(len).trimmed();
            s.remove(0, len + pattern_len);
            return ret;
        };

        info.address = take_before(s, ":");
        info.function = take_before(s, " at ");
        info.location = take_before(s, "");

        info.location.replace("c:/Devel/Workspace/", "", Qt::CaseInsensitive);
        info.location.replace("c:/Devel/Tools/", "", Qt::CaseInsensitive);

        ret << info;
    }
    return ret;
}

QString CrashHandler::StackInfo::prettyPrint(bool has_horizontal_scroll) const
{
    auto length = address.size() + function.size() + location.size();
    auto has_location = !location.isEmpty();
    if (length <= 80)
    {
        auto ret = address + ": " + function;
        if (has_location)
        {
            ret.append(" at " + location);
        }
        return ret;
    }

    static auto split_length = [] (const QString& s) {
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
        ret.append("\t" + function + "\n");
        if (has_location)
        {
            ret.append("\t" + location);
        }
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
