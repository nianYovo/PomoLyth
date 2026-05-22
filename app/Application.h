#pragma once

#include <memory>
#include <QObject>
#include <QSystemTrayIcon>
#include "ai/IAiClient.h"
#include "ai/MockAiClient.h"
#include "ai/OpenAiClient.h"
#include "ai/ReviewGenerator.h"
#include "ai/TaskPlanner.h"
#include "core/AchievementSystem.h"
#include "core/EventBus.h"
#include "core/FocusSessionManager.h"
#include "core/PetStateMachine.h"
#include "core/PomodoroTimer.h"
#include "monitor/FocusMonitor.h"
#include "storage/JsonStorage.h"
#include "storage/SQLiteStorage.h"
#include "ui/MainWindow.h"

class Application : public QObject {
    Q_OBJECT

public:
    explicit Application(QObject* parent = nullptr);
    bool initialize();
    void show();

private:
    void loadTheme();
    void setupTray();
    void setupNotifications();
    void setupAiClient();

    EventBus m_eventBus;
    PomodoroTimer m_timer;
    std::unique_ptr<IAiClient> m_aiClient;
    std::unique_ptr<TaskPlanner> m_taskPlanner;
    std::unique_ptr<ReviewGenerator> m_reviewGenerator;
    SQLiteStorage m_storage;
    JsonStorage m_jsonStorage;
    AppConfig m_config;
    AchievementSystem m_achievementSystem;
    std::unique_ptr<FocusMonitor> m_focusMonitor;
    std::unique_ptr<FocusSessionManager> m_sessionManager;
    std::unique_ptr<PetStateMachine> m_petStateMachine;
    std::unique_ptr<MainWindow> m_mainWindow;
    std::unique_ptr<QSystemTrayIcon> m_trayIcon;
};
