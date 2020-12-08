#pragma once

#include <string>
#include <vector>
#include <QString>

class CrashHandler
{
public:
    static void Attach(void);
    static void BreakDebugger(void);
protected:

    class StackInfo
    {
    public:
        QString address;
        QString function;
        QString location;
        QString prettyPrint(
            bool has_horizontal_scroll=false,
            bool rich_text=false) const;
    };
    using Stack = QList<StackInfo>;

    QStringList addr2line(const std::vector<void*>& addr);
    Stack parseStack(const QStringList& stack);
    bool hasAlreadyCrashed(void);

    void showDialog(const std::string& error, const Stack& stack);
    void showTerminal(const std::string& error, const Stack& stack);

    virtual bool isDebuggerAttached(void) const = 0;
    virtual void breakDebugger(bool force=false) const = 0;

};
