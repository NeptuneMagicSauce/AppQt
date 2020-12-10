#pragma once

#include <QString>
#include <QList>

namespace Utils::StackTrace
{
    struct StackInfo
    {
        QString address;
        QString function;
        QString location;
    };
    using Stack = QList<StackInfo>;

#if _WIN64
    Stack fromContext(void* exception); // not used
#endif
    Stack getCurrent(void);
}
