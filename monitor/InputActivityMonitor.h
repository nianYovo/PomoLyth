#pragma once

#include <QObject>
#include <QTimer>
#include "core/EventBus.h"

class InputActivityMonitor : public QObject {
    Q_OBJECT

public:
    explicit InputActivityMonitor(EventBus& eventBus, QObject* parent = nullptr);

    void startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const;

    int activityCount() const;
    int maxIdleSeconds() const;
    int currentIdleSeconds() const;
    void setIdleWarningSeconds(int seconds);

signals:
    void activityChanged(int activityCount, int idleSeconds);
    void idleWarning(int idleSeconds);

private:
    int readIdleSeconds() const;
    void poll();

    EventBus& m_eventBus;
    QTimer m_timer;
    int m_lastIdleSeconds = 0;
    int m_activityCount = 0;
    int m_maxIdleSeconds = 0;
    int m_idleWarningSeconds = 120;
    bool m_idleWarningSent = false;
};
