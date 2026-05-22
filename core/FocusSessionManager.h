#pragma once

#include <QObject>
#include <QFutureWatcher>
#include "ai/ReviewGenerator.h"
#include "ai/TaskPlanner.h"
#include "core/AchievementSystem.h"
#include "core/EventBus.h"
#include "core/PomodoroTimer.h"
#include "models/PetProfile.h"
#include "monitor/FocusMonitor.h"
#include "storage/SQLiteStorage.h"

class FocusSessionManager : public QObject {
    Q_OBJECT

public:
    FocusSessionManager(
        EventBus& eventBus,
        PomodoroTimer& timer,
        TaskPlanner& taskPlanner,
        ReviewGenerator& reviewGenerator,
        FocusMonitor& focusMonitor,
        SQLiteStorage& storage,
        AchievementSystem& achievementSystem,
        QObject* parent = nullptr);

    FocusTask planTask(const QString& input, int minutes);
    void requestPlan(const QString& input, int minutes);
    void startSession(const FocusTask& task);
    void pauseSession();
    void resumeSession();
    void cancelSession();
    void submitReview(const QStringList& completedGoals, const QStringList& problems, int selfRating);
    void submitReviewAsync(const QStringList& completedGoals, const QStringList& problems, int selfRating);
    void setBreakMinutes(int minutes);

    FocusSession currentSession() const;
    PetProfile petProfile() const;

signals:
    void planGenerated(const FocusTask& task);
    void sessionCompleted(const FocusSession& session);
    void sessionSaved(const FocusSession& session, const QStringList& achievements);
    void aiTaskStarted(const QString& label);
    void aiTaskFinished(const QString& label);
    void breakStarted(int minutes);
    void breakFinished();
    void distractionCountChanged(int count);

private:
    void onTimerCompleted();
    void onAppEvent(const AppEvent& event);

    EventBus& m_eventBus;
    PomodoroTimer& m_timer;
    TaskPlanner& m_taskPlanner;
    ReviewGenerator& m_reviewGenerator;
    FocusMonitor& m_focusMonitor;
    SQLiteStorage& m_storage;
    AchievementSystem& m_achievementSystem;
    FocusTask m_currentTask;
    FocusSession m_currentSession;
    PetProfile m_petProfile;
    bool m_hasActiveSession = false;
    bool m_hasActiveBreak = false;
    bool m_aiBusy = false;
    int m_breakMinutes = 5;
};
