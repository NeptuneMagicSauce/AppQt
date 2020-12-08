#pragma once

#include "CrashHandler.hpp"

class CrashHandlerWin64: public CrashHandler
{
public:
    CrashHandlerWin64(void);
protected:
    virtual bool isDebuggerAttached(void) const override;
    virtual void breakDebugger(bool force=false) const override;
    virtual void attachGDB(void) const override;
    virtual bool canAttachGDB(void) const override;
private:
    friend class CrashHandlerWin64Impl;
    void handle(void* exception);
    QStringList walkStack(void* exception);
};
