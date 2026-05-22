#pragma once

#include <QMainWindow>
#include "ai/ReviewGenerator.h"
#include "core/FocusSessionManager.h"
#include "core/PetStateMachine.h"
#include "models/FocusTask.h"
#include "monitor/FocusMonitor.h"
#include "storage/JsonStorage.h"

class DashboardWindow;
class QCloseEvent;
class QLabel;
class PetWidget;
class PetWindow;
class QProgressBar;
class QTextEdit;
class QLineEdit;
class TimerPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(
        FocusSessionManager& sessionManager,
        PomodoroTimer& timer,
        PetStateMachine& petStateMachine,
        SQLiteStorage& storage,
        JsonStorage& jsonStorage,
        FocusMonitor& focusMonitor,
        ReviewGenerator& reviewGenerator,
        const AppConfig& config,
        QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void buildUi();
    void applyStyle();
    void requestPlan();
    void startSession();
    void cancelSessionWithConfirm();
    void showDashboard();
    void togglePetWindow();
    void showSettings();
    void showReview(const FocusSession& session);
    void updatePlanView(const FocusTask& task);
    void updatePetProfileView();

    FocusSessionManager& m_sessionManager;
    PomodoroTimer& m_timer;
    PetStateMachine& m_petStateMachine;
    SQLiteStorage& m_storage;
    JsonStorage& m_jsonStorage;
    FocusMonitor& m_focusMonitor;
    ReviewGenerator& m_reviewGenerator;
    AppConfig m_config;
    QStringList m_blacklist;
    QStringList m_whitelist;
    FocusTask m_currentTask;

    QLineEdit* m_taskInput = nullptr;
    QTextEdit* m_planView = nullptr;
    QLabel* m_statusLabel = nullptr;
    QLabel* m_petProfileLabel = nullptr;
    QProgressBar* m_expProgress = nullptr;
    TimerPanel* m_timerPanel = nullptr;
    PetWidget* m_petWidget = nullptr;
    PetWindow* m_petWindow = nullptr;
    DashboardWindow* m_dashboard = nullptr;
};
