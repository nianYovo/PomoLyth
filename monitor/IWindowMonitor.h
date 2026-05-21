#pragma once

#include <QString>

class IWindowMonitor {
public:
    virtual ~IWindowMonitor() = default;
    virtual QString activeWindowTitle() const = 0;
    virtual QString activeProcessName() const = 0;
};
