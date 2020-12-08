#include "CrashHandler.hpp"

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
#include "Utils.hpp"
#if _WIN64
#include "CrashHandlerWindows.hpp"
#endif

using std::string;
using std::vector;

// TODO port FloatingPointExceptions::Disabler, disable fpe in excpt handler

// TODO other threads: do they need to attach crash handler and disable FPE ?

namespace CrashHandlerImpl
{
    CrashHandler* instance = nullptr;
};

void CrashHandler::Attach(void)
{
#if _WIN64
    CrashHandlerImpl::instance = new CrashHandlerWin64;
#else
#warning "CrashHandler not implemened"
#endif
}

void CrashHandler::BreakDebugger(void)
{
    if (CrashHandlerImpl::instance)
    {
        CrashHandlerImpl::instance->breakDebugger(true);
    }
}

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
    if (stack.isEmpty())
    {
        return;
    }
    std::cerr << "Stack Trace" << std::endl;
    auto qdebug = qDebug();
    qdebug.noquote();
    qdebug.nospace();
    for (const auto& s : stack)
    {
        qdebug << s.prettyPrint() << "\n";
    }
}

void CrashHandler::showDialog(const string& error, const Stack& stack)
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
    layout_root.addWidget(widgetCentered({&error_label}));

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


    auto has_gdb = QProcess::startDetached("gdb", { "-q", "-ex", "quit" });
    button_gdb.setEnabled(has_gdb);

    QLabel stack_label;
    auto stack_font = QFont{"Consolas"};
    stack_font.setPointSizeF(8.5f);
    stack_label.setFont(stack_font);
    stack_label.setTextFormat(Qt::RichText);
    QString stack_text;
    stack_text += "<b>Process ID</b> " + pid_string + "<br><br>";
    stack_text += "<b>Stack Trace</b><br><br>";
    for (const auto& s: stack)
    {
        stack_text += s.prettyPrint(true, true) + "<br>";
    }
    stack_label.setText(stack_text);
    QScrollArea stack_area;
    stack_area.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    stack_area.setWidget(&stack_label);
    layout_root.addWidget(&stack_area);
    dialog.resize(800, 500);
    dialog.exec();

    if (deferred_gdb)
    {
        QProcess::startDetached(
            "gdb",
            { "-quiet" ,
              "-ex", "\"attach " + pid_string + "\"",
              // continue on first breakpoint because
              // windows breaks on attach at DbgUiRemoteBreakin
              // TODO linux check if continue command next line should be removed
              // TODO linux gdb will need to be inside a terminal
              "-ex", "continue"
            });

        for (int i=0; i<100; ++i)
        {
            if (isDebuggerAttached())
            {
                breakDebugger();
                break;
            } else {
                if (i > 0)
                {
                    qDebug() << "GDB not ready";
                }
                QThread::msleep(100);
            }
        }
    }
}

QStringList CrashHandler::addr2line(const vector<void*>& addr)
{
    QProcess p;
    QStringList args =
        {
            "-C", // --demangle
            // "-s" // --basenames
            "-a", // --addresses
            "-f", // --functions
            "-p", // --pretty-print
            "-e",
            QCoreApplication::applicationFilePath(),
        };
    for (auto& a: addr)
    {
        args << QString::fromStdString(Utils::toHexa(a));
    }

    p.start("addr2line", args);
    if (!p.waitForStarted())
    {
        QStringList ret;
        ret << "addr2line failed";
        for (auto& a: addr)
        {
            ret << QString::fromStdString(Utils::toHexa(a));
        }
        return ret;
    }
    p.waitForFinished();
    // TODO check split(\r\n) on Linux
    return QString(p.readAll()).split("\r\n", Qt::SkipEmptyParts);
}

CrashHandler::Stack CrashHandler::parseStack(const QStringList& stack)
{
    Stack ret;
    for (auto s: stack)
    {
        StackInfo info;

        static auto takeBefore = [] (QString& s, const QString& pattern) {
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

        info.address = takeBefore(s, ":");
        info.function = takeBefore(s, " at ");
        info.location = takeBefore(s, "");

        static auto removePrefix = [] (QString& s, const QString& prefix) {
            if (s.startsWith(prefix, Qt::CaseInsensitive))
            {
                s.remove(0, prefix.size());
            }
        };
        removePrefix(info.location, "c:/Devel/Workspace/");
        removePrefix(info.location, "c:/Devel/Tools/");

        ret << info;
    }
    return ret;
}

QString CrashHandler::StackInfo::prettyPrint(bool has_horizontal_scroll, bool rich_text) const
{
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
        return mark.first + item + mark.second;// + "<br>";// + "\n";
    };

    auto a = formatItem(address, Type::Address, rich_text);
    auto f = formatItem(function, Type::Function, rich_text);
    auto l = formatItem(location, Type::Location, rich_text);

    static const std::map<bool, QString> brs =
        {
            { true, "<br>" },
            { false, "\n" },
        };
    static const std::map<bool, QString> tabs =
        {
            { true, "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" },
            { false, "\t" },
        };

    const auto br = brs.at(rich_text);
    const auto tab = tabs.at(rich_text);
    static const QString location_prefix = "at ";

    auto has_location = !location.isEmpty();
    auto ret = "[" + a + "] ";
    if (address.size() + function.size() + location.size() <= 80)
    {
        ret += f + " ";
        if (has_location)
        {
            ret += location_prefix + l;
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
            ret += tab + location_prefix + l;
        }
    } else {
        ret += br;
        for (auto& f: splitLength(f))
        {
            ret += tab + f + br;
        }
        auto first_location = true;
        for (auto& l: splitLength(l))
        {
            ret += tab;
            if (first_location)
            {
                ret += location_prefix;
                first_location = false;
            }
            ret += l + br;
        }
    }
    return ret.trimmed();
}
