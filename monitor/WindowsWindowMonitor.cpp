#include "monitor/WindowsWindowMonitor.h"

#ifdef Q_OS_WIN
#include <QFileInfo>
#include <qt_windows.h>
#include <psapi.h>
#endif

QString WindowsWindowMonitor::activeWindowTitle() const {
#ifdef Q_OS_WIN
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) {
        return {};
    }

    wchar_t title[512] = {};
    GetWindowTextW(hwnd, title, 512);
    return QString::fromWCharArray(title);
#else
    return {};
#endif
}

QString WindowsWindowMonitor::activeProcessName() const {
#ifdef Q_OS_WIN
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) {
        return {};
    }

    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (!process) {
        return {};
    }

    wchar_t path[MAX_PATH] = {};
    QString name;
    if (GetModuleFileNameExW(process, nullptr, path, MAX_PATH) > 0) {
        name = QFileInfo(QString::fromWCharArray(path)).fileName();
    }
    CloseHandle(process);
    return name;
#else
    return {};
#endif
}
