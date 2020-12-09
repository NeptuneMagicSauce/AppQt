#include "CrashDialog.hpp"

#include <iostream>
#include <map>
#include <QDebug>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QScrollArea>
#include <QTimer>
#include <QCoreApplication>
#include <QProcess>
#include <QThread>
#include <QApplication>

using Stack = CrashHandler::Stack;
using StackInfo = CrashHandler::StackInfo;

class CrashDialogImpl
{
public:
    void showTerminal(
        const QString& error,
        const Stack& stack,
        const QString& location);
    void showDialog(
        const QString& error,
        const Stack& stack,
        const QString& location);
    QString prettyPrintStack(
        const StackInfo& info,
        bool has_horizontal_scroll=false,
        bool rich_text=false);
    QString removeFilePathSpecifics(const QString& location)
    {
        static auto withoutPrefix = [] (const QString& s, const QString& prefix) {
            if (s.startsWith(prefix, Qt::CaseInsensitive))
            {
                return s.right(s.size() - prefix.size());
            }
            return s;
        };
        return withoutPrefix(withoutPrefix(location,
                             "c:/Devel/Tools/"),
                             "c:/Devel/Workspace/");
    }
    QString prettyPrintLocation(const CrashDialog::Location& location)
    {
        QString ret;
        if (location.line > 0)
        {
            ret.append(prefix_function);
            ret.append(location.function);
            ret.append("\n");
            ret.append(prefix_location);
            ret.append(removeFilePathSpecifics(location.file));
            ret.append(":");
            ret.append(QString::number(location.line));
            ret.append("\n");
        }
        return ret;
    }
    static constexpr const char* prefix_function = "in ";
    static constexpr const char* prefix_location = "at ";
} impl_cd;

void CrashDialog::panic(
    const QString& error,
    const Stack& stack,
    const Location& location)
{
    auto location_parsed = impl_cd.prettyPrintLocation(location);

    impl_cd.showTerminal(error, stack, location_parsed);
    impl_cd.showDialog(error, stack, location_parsed);

    // exit as fast as possible, no deferral
    // because we can't expect other systems to work
    // QCoreApplication::quit() with the event loop does not work
    std::exit(1);
}

void CrashDialogImpl::showTerminal(
    const QString& error,
    const Stack& stack,
    const QString& location)
{
    // TODO produce core dump ? with or without qFatal
    auto qdebug = QTextStream(stdout);
    qdebug << error << "\n";
    if (location.size())
    {
        qdebug << "\n" << location << "\n";
    }
    if (stack.isEmpty())
    {
        return;
    }
    qdebug << "Stack Trace\n";
    for (const auto& s : stack)
    {
        qdebug << impl_cd.prettyPrintStack(s) << "\n";
    }
    qdebug.flush();
}

void CrashDialogImpl::showDialog(
    const QString& error,
    const Stack& stack,
    const QString& location)
{
    auto widgetCentered = [] (QList<QWidget*> widgets)
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
        for (auto* w: widgets)
        {
            layout->addWidget(w);
            if (w != widgets.back())
            {
                layout->addSpacing(20);
            }
        }
        layout->addWidget(spacer_right);
        return base;
    };

    if (qApp == nullptr)
    {
        new QApplication(*(new int(0)), (char**)nullptr);
    }

    QDialog dialog {
        nullptr,
        Qt::WindowTitleHint
        | Qt::WindowSystemMenuHint
        | Qt::WindowMaximizeButtonHint
        | Qt::WindowCloseButtonHint };
    dialog.setWindowTitle("Crash");
    QVBoxLayout layout_root;
    dialog.setLayout(&layout_root);
    QLabel error_label { error };
    error_label.setAlignment(Qt::AlignHCenter);
    layout_root.addWidget(widgetCentered({&error_label}));
    QLabel location_label { location };
    if (location.size())
    {
        layout_root.addWidget(widgetCentered({&location_label}));
    }

    QPushButton button_quit { "Quit" };
    button_quit.setDefault(true);
    QObject::connect(&button_quit, &QPushButton::released, [&dialog](){
        dialog.accept();
    });
    auto pid_string = QString::number(QCoreApplication::applicationPid());
    QPushButton button_gdb { "Attach GDB" };
    auto deferred_gdb = false;
    QObject::connect(&button_gdb, &QPushButton::released, [&deferred_gdb, &dialog](){
        deferred_gdb = true;
        dialog.accept();
    });
    layout_root.addWidget(widgetCentered({&button_quit, &button_gdb}));
    button_gdb.setEnabled(CrashHandler::instance().canAttachDebugger());

    auto& stack_label = *(new QLabel);
    // stack_label will be destroyed by its parent stack_area -> dyn alloc
    auto stack_font = QFont{"Consolas"};
    stack_font.setPointSizeF(8.5f);
    stack_label.setFont(stack_font);
    stack_label.setTextFormat(Qt::RichText);
    QString stack_text;
    stack_text += "<b>Process ID</b> " + pid_string + "<br><br>";
    stack_text += "<b>Stack Trace</b><br><br>";
    for (const auto& s: stack)
    {
        stack_text += impl_cd.prettyPrintStack(s, true, true) + "<br>";
    }
    stack_label.setText(stack_text);

    QScrollArea stack_area;
    stack_area.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    stack_area.setWidget(&stack_label);
    layout_root.addWidget(&stack_area);

    // TODO if no stack or short stack -> smaller size in both dimensions

    dialog.resize(800, 500);
    dialog.exec();

    if (deferred_gdb)
    {
        CrashHandler::instance().attachDebugger();
    }
}

QString CrashDialogImpl::prettyPrintStack(
    const StackInfo& info,
    bool has_horizontal_scroll,
    bool rich_text)
{
    auto& address = info.address;
    auto& function = info.function;
    auto location = info.location;

    enum struct Type: int { Address, Function, Unknown, Location };
    static auto formatItem = [] (const QString& item, Type type, bool rich_text) {
        if (!rich_text)
        {
            return item;
        }
        static auto marker = [] (const QString& weight, const QString& color_hexa) {
            std::pair<QString, QString> ret;
            auto has_weight = weight.size() == 1;
            auto has_color = color_hexa.size() == 6;
            if (has_weight)
            {
                ret.first = "<" + weight + ">";
            }
            if (has_color)
            {
                ret.first += "<span b style=\"color:#" + color_hexa + "\";>";
                ret.second = "</span>";
            }
            if (has_weight)
            {
                ret.second += "</" + weight + ">";
            }
            return ret;
        };

        static const std::map<Type, std::pair<QString, QString>> marks =
        {
            { Type::Address , marker("b", "0075DA") },
            { Type::Function, marker("b", "DB3DED") },
            { Type::Unknown,  marker("i", ""      ) },
            { Type::Location, marker("b", "007F0E") },
        };
        if (type == Type::Function && item.startsWith("??"))
        {
            type = Type::Unknown;
        }
        auto& mark = marks.at(type);
        auto item_safe = item;
        item_safe.replace("&", "&amp;");
        item_safe.replace("<", "&lt;");
        item_safe.replace(">", "&gt;");
        item_safe.replace("\"", "&quot;");
        item_safe.replace("'", "&apos;");
        return mark.first + item_safe + mark.second;
    };

    location = impl_cd.removeFilePathSpecifics(location);

    auto a = formatItem(address, Type::Address, rich_text);
    auto f = formatItem(function, Type::Function, rich_text);
    auto l = formatItem(location, Type::Location, rich_text);

    f = CrashDialogImpl::prefix_function + f;
    l = CrashDialogImpl::prefix_location + l;

    static const std::map<bool, QString> brs =
        {
            { true, "<br>" },
            { false, "\n" },
        };
    static const std::map<bool, QString> tabs =
        {
            { true, "&nbsp;&nbsp;&nbsp;&nbsp;" },
            { false, "\t" },
        };

    const auto br = brs.at(rich_text);
    const auto tab = tabs.at(rich_text);

    auto has_location = !location.isEmpty();
    auto ret = QString{};
    if (address.size())
    {
        ret += "[" + a + "] ";
    }
    if (address.size() + function.size() + location.size() <= 80)
    {
        ret += f + " ";
        if (has_location)
        {
            ret += l;
        }
        return ret;
    }

    static auto splitLength = [] (const QString& s) {
        QStringList ret;
        auto tmp = s;
        while (!tmp.isEmpty())
        {
            ret << tmp.left(80);
            tmp.remove(0, 80);
        }
        return ret;
    };

    if (has_horizontal_scroll)
    {
        ret += f + br;
        if (has_location)
        {
            ret += tab + l;
        }
    } else {
        ret += br;
        constexpr const char* prefix_empty = "   ";
        auto first_f = true;
        for (auto& f: splitLength(f))
        {
            ret += tab;
            if (!first_f)
            {
                ret += prefix_empty;
            }
            first_f = false;
            ret += f + br;
        }
        auto first_l = true;
        for (auto& l: splitLength(l))
        {
            ret += tab;
            if (!first_l)
            {
                ret += prefix_empty;
            }
            first_l = false;
            ret += l + br;
        }
    }
    return ret.trimmed();
}
