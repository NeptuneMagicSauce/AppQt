#pragma once

#include "CrashHandler.hpp"

class CrashHandlerWin64: public CrashHandler
{
public:
    CrashHandlerWin64(void);
protected:
    virtual bool canAttachDebugger(void) const override;
    virtual bool isDebuggerAttached(void) const override;
    virtual void attachDebugger(void) const override;
    virtual void breakDebugger(bool force=false) const override;
private:
    friend class CrashHandlerWin64Impl;
    void handle(void* exception);
    QStringList walkStack(void* exception);
};
