#include "monitor/InputActivityMonitor.h"

#include <algorithm>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

InputActivityMonitor::InputActivityMonitor(EventBus& eventBus, QObject* parent)
    : QObject(parent), m_eventBus(eventBus) {
    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, this, &InputActivityMonitor::poll);
}

void InputActivityMonitor::startMonitoring() {
    m_activityCount = 0;
    m_maxIdleSeconds = 0;
    m_lastIdleSeconds = readIdleSeconds();
    m_idleWarningSent = false;
    m_timer.start();
}

void InputActivityMonitor::stopMonitoring() {
    m_timer.stop();
}

bool InputActivityMonitor::isMonitoring() const {
    return m_timer.isActive();
}

int InputActivityMonitor::activityCount() const {
    return m_activityCount;
}

int InputActivityMonitor::maxIdleSeconds() const {
    return m_maxIdleSeconds;
}

int InputActivityMonitor::currentIdleSeconds() const {
    return readIdleSeconds();
}

void InputActivityMonitor::setIdleWarningSeconds(int seconds) {
    m_idleWarningSeconds = std::max(10, seconds);
}

int InputActivityMonitor::readIdleSeconds() const {
#ifdef Q_OS_WIN
    LASTINPUTINFO info;
    info.cbSize = sizeof(LASTINPUTINFO);
    if (!GetLastInputInfo(&info)) {
        return 0;
    }
    const DWORD elapsedMs = GetTickCount() - info.dwTime;
    return static_cast<int>(elapsedMs / 1000);
#else
    return 0;
#endif
}

void InputActivityMonitor::poll() {
    const int idleSeconds = readIdleSeconds();
    m_maxIdleSeconds = std::max(m_maxIdleSeconds, idleSeconds);

    if (idleSeconds < m_lastIdleSeconds) {
        ++m_activityCount;
        m_idleWarningSent = false;

        AppEvent event;
        event.type = AppEventType::InputActivityDetected;
        event.payload = QString::number(m_activityCount);
        event.data.insert("idleSeconds", idleSeconds);
        m_eventBus.publish(event);

        emit activityChanged(m_activityCount, idleSeconds);
    }

    if (idleSeconds >= m_idleWarningSeconds && !m_idleWarningSent) {
        m_idleWarningSent = true;

        AppEvent event;
        event.type = AppEventType::InputIdleDetected;
        event.payload = QString::number(idleSeconds);
        event.data.insert("idleSeconds", idleSeconds);
        m_eventBus.publish(event);

        emit idleWarning(idleSeconds);
    }

    m_lastIdleSeconds = idleSeconds;
}
