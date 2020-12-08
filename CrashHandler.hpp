#pragma once

#include <string>
#include <vector>
#include <QString>
#include <QList>

class CrashHandler
{
public:
    // TODO rename first Attach()
    // TODO move these 4 static functions to instance functions
    static void Attach(void);
    static void BreakDebugger(void);
    static void AttachGDB(void);
    static bool CanAttachGDB(void);

    class StackInfo
    {
    public:
        QString address;
        QString function;
        QString location;
    };
    using Stack = QList<StackInfo>;

protected:
    QStringList addr2line(const std::vector<void*>& addr);
    Stack parseStack(const QStringList& stack);
    bool hasAlreadyCrashed(void);
    void finishPanic(const std::string& error, const Stack& stack);

protected:
    virtual bool isDebuggerAttached(void) const = 0;
    virtual void breakDebugger(bool force=false) const = 0;
    virtual void attachGDB(void) const = 0;
    virtual bool canAttachGDB(void) const  = 0;
};
