#pragma once

namespace Utils::Debugger
{
    bool canAttachDebugger(void);
    bool isDebuggerAttached(void);
    void attachDebugger(void);
    void breakDebugger(void);
}
