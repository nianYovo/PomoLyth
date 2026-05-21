#pragma once

#include <QObject>
#include <QSet>
#include <QTimer>
#include <memory>
#include "core/EventBus.h"
#include "models/DistractionEvent.h"
#include "monitor/IWindowMonitor.h"

class FocusMonitor : public QObject {
    Q_OBJECT

public:
    FocusMonitor(EventBus& eventBus, std::unique_ptr<IWindowMonitor> windowMonitor, QObject* parent = nullptr);

    void startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const;
    int switchCount() const;
    QVector<DistractionEvent> events() const;
    void setBlacklist(const QStringList& keywords);
    void setWhitelist(const QStringList& keywords);
    void setIntervalSeconds(int seconds);

private:
    void checkWindow();
    bool matchesWhitelist(const QString& title, const QString& process) const;
    bool matchesBlacklist(const QString& title, const QString& process) const;

    EventBus& m_eventBus;
    std::unique_ptr<IWindowMonitor> m_windowMonitor;
    QTimer m_timer;
    QString m_lastTitle;
    int m_switchCount = 0;
    QVector<DistractionEvent> m_events;
    QStringList m_blacklist = {"bilibili", "youtube", "steam", "tiktok", "douyin", "game", "video"};
    QStringList m_whitelist;
};
