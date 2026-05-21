#pragma once

#include "monitor/IWindowMonitor.h"

class WindowsWindowMonitor : public IWindowMonitor {
public:
    QString activeWindowTitle() const override;
    QString activeProcessName() const override;
};
