#include "monitor/FocusMonitor.h"

#include <algorithm>

FocusMonitor::FocusMonitor(EventBus& eventBus, std::unique_ptr<IWindowMonitor> windowMonitor, QObject* parent)
    : QObject(parent), m_eventBus(eventBus), m_windowMonitor(std::move(windowMonitor)) {
    m_timer.setInterval(3000);
    connect(&m_timer, &QTimer::timeout, this, &FocusMonitor::checkWindow);
}

void FocusMonitor::startMonitoring() {
    m_switchCount = 0;
    m_events.clear();
    m_lastTitle.clear();
    m_timer.start();
}

void FocusMonitor::stopMonitoring() {
    m_timer.stop();
}

bool FocusMonitor::isMonitoring() const {
    return m_timer.isActive();
}

int FocusMonitor::switchCount() const {
    return m_switchCount;
}

QVector<DistractionEvent> FocusMonitor::events() const {
    return m_events;
}

void FocusMonitor::setBlacklist(const QStringList& keywords) {
    m_blacklist = keywords;
}

void FocusMonitor::setWhitelist(const QStringList& keywords) {
    m_whitelist = keywords;
}

void FocusMonitor::setIntervalSeconds(int seconds) {
    m_timer.setInterval(std::max(1, seconds) * 1000);
}

void FocusMonitor::checkWindow() {
    if (!m_windowMonitor) {
        return;
    }

    const QString title = m_windowMonitor->activeWindowTitle();
    const QString process = m_windowMonitor->activeProcessName();
    if (!title.isEmpty() && title != m_lastTitle) {
        ++m_switchCount;
        m_lastTitle = title;
    }

    if (matchesWhitelist(title, process) || !matchesBlacklist(title, process)) {
        return;
    }

    DistractionEvent distraction;
    distraction.windowTitle = title;
    distraction.processName = process;
    distraction.severity = m_events.size() >= 2 ? 3 : 1;
    m_events.append(distraction);

    AppEvent event;
    event.type = AppEventType::DistractionDetected;
    event.payload = title;
    event.data.insert("process", process);
    event.data.insert("severity", distraction.severity);
    m_eventBus.publish(event);
}

bool FocusMonitor::matchesWhitelist(const QString& title, const QString& process) const {
    const QString haystack = QString("%1 %2").arg(title, process).toLower();
    for (const QString& keyword : m_whitelist) {
        if (!keyword.trimmed().isEmpty() && haystack.contains(keyword.trimmed().toLower())) {
            return true;
        }
    }
    return false;
}

bool FocusMonitor::matchesBlacklist(const QString& title, const QString& process) const {
    const QString haystack = QString("%1 %2").arg(title, process).toLower();
    for (const QString& keyword : m_blacklist) {
        if (!keyword.trimmed().isEmpty() && haystack.contains(keyword.trimmed().toLower())) {
            return true;
        }
    }
    return false;
}
