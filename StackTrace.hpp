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

    Stack getCurrent(void);
}
