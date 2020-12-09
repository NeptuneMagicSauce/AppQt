#pragma once

#include <vector>
#include <QString>
#include <QList>

class CrashHandler
{
public:
    static CrashHandler& instance(void);

    class StackInfo
    {
    public:
        QString address;
        QString function;
        QString location;
    };
    using Stack = QList<StackInfo>;

    virtual bool canAttachDebugger(void) const  = 0;
    virtual bool isDebuggerAttached(void) const = 0;
    virtual void attachDebugger(void) const = 0;
    virtual void breakDebugger(bool force=false) const = 0;
    virtual Stack currentStack(void) const = 0;

protected:
    QStringList addr2line(const std::vector<void*>& addr) const;
    Stack parseStack(const QStringList& stack) const;
    bool hasAlreadyCrashed(void) const;
    void finishPanic(const QString& error, const Stack& stack) const;

};
