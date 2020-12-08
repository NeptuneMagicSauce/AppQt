#pragma once

#include <string>
#include <vector>
#include <QString>
#include <QList>

class CrashHandler
{
public:
    static void install(void);
    static CrashHandler& instance(void);

    virtual bool canAttachDebugger(void) const  = 0;
    virtual bool isDebuggerAttached(void) const = 0;
    virtual void attachDebugger(void) const = 0;
    virtual void breakDebugger(bool force=false) const = 0;

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

};
