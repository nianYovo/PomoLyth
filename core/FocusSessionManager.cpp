#include "core/FocusSessionManager.h"

#include <algorithm>
#include <QtConcurrent>

FocusSessionManager::FocusSessionManager(
    EventBus& eventBus,
    PomodoroTimer& timer,
    TaskPlanner& taskPlanner,
    ReviewGenerator& reviewGenerator,
    FocusMonitor& focusMonitor,
    InputActivityMonitor& inputActivityMonitor,
    SQLiteStorage& storage,
    AchievementSystem& achievementSystem,
    QObject* parent)
    : QObject(parent),
      m_eventBus(eventBus),
      m_timer(timer),
      m_taskPlanner(taskPlanner),
      m_reviewGenerator(reviewGenerator),
      m_focusMonitor(focusMonitor),
      m_inputActivityMonitor(inputActivityMonitor),
      m_storage(storage),
      m_achievementSystem(achievementSystem),
      m_petProfile(storage.loadPetProfile()) {
    connect(&m_timer, &PomodoroTimer::completed, this, &FocusSessionManager::onTimerCompleted);
    connect(&m_eventBus, &EventBus::eventPublished, this, &FocusSessionManager::onAppEvent);
}

FocusTask FocusSessionManager::planTask(const QString& input, int minutes) {
    m_currentTask = m_taskPlanner.generatePlan(input, minutes);

    AppEvent event;
    event.type = AppEventType::AiPlanGenerated;
    event.payload = input;
    m_eventBus.publish(event);

    emit planGenerated(m_currentTask);
    return m_currentTask;
}

void FocusSessionManager::requestPlan(const QString& input, int minutes) {
    if (m_aiBusy) {
        return;
    }
    m_aiBusy = true;
    emit aiTaskStarted("Planning");

    auto* watcher = new QFutureWatcher<FocusTask>(this);
    connect(watcher, &QFutureWatcher<FocusTask>::finished, this, [this, watcher, input]() {
        m_currentTask = watcher->result();
        watcher->deleteLater();
        m_aiBusy = false;

        AppEvent event;
        event.type = AppEventType::AiPlanGenerated;
        event.payload = input;
        m_eventBus.publish(event);

        emit planGenerated(m_currentTask);
        emit aiTaskFinished("Planning");
    });

    watcher->setFuture(QtConcurrent::run([this, input, minutes]() {
        return m_taskPlanner.generatePlan(input, minutes);
    }));
}

void FocusSessionManager::startSession(const FocusTask& task) {
    m_currentTask = task;
    m_currentSession = FocusSession{};
    m_currentSession.task = task.originalInput;
    m_currentSession.plannedMinutes = task.estimatedMinutes;
    m_currentSession.createdAt = QDateTime::currentDateTime();
    m_hasActiveSession = true;

    m_focusMonitor.startMonitoring();
    m_inputActivityMonitor.startMonitoring();
    m_timer.start(task.estimatedMinutes);

    AppEvent event;
    event.type = AppEventType::TimerStarted;
    event.payload = task.originalInput;
    m_eventBus.publish(event);
}

void FocusSessionManager::pauseSession() {
    m_timer.pause();
    AppEvent event;
    event.type = AppEventType::TimerPaused;
    m_eventBus.publish(event);
}

void FocusSessionManager::resumeSession() {
    m_timer.resume();
    AppEvent event;
    event.type = AppEventType::TimerResumed;
    m_eventBus.publish(event);
}

void FocusSessionManager::cancelSession() {
    m_focusMonitor.stopMonitoring();
    m_inputActivityMonitor.stopMonitoring();
    m_timer.stop();
    m_hasActiveSession = false;

    AppEvent event;
    event.type = AppEventType::TimerCancelled;
    m_eventBus.publish(event);
}

void FocusSessionManager::submitReview(const QStringList& completedGoals, const QStringList& problems, int selfRating) {
    if (m_currentSession.task.isEmpty()) {
        return;
    }

    m_currentSession.completedGoals = completedGoals;
    m_currentSession.problems = problems;
    m_currentSession.problems.append(QString("Self rating: %1/5").arg(selfRating));
    m_currentSession = m_reviewGenerator.generateReview(m_currentSession);

    const QStringList achievements = m_achievementSystem.update(m_currentSession, m_petProfile);
    m_storage.saveFocusSession(m_currentSession);
    m_storage.saveDistractionEvents(m_currentSession.id, m_focusMonitor.events());
    m_storage.savePetProfile(m_petProfile);
    for (const QString& achievement : achievements) {
        m_storage.saveAchievement(achievement);
    }

    AppEvent reviewEvent;
    reviewEvent.type = AppEventType::AiReviewGenerated;
    reviewEvent.payload = m_currentSession.aiSummary;
    m_eventBus.publish(reviewEvent);

    if (!achievements.isEmpty()) {
        AppEvent levelEvent;
        levelEvent.type = AppEventType::PetLevelUp;
        levelEvent.payload = achievements.join(", ");
        m_eventBus.publish(levelEvent);

        for (const QString& achievement : achievements) {
            AppEvent unlockedEvent;
            unlockedEvent.type = AppEventType::AchievementUnlocked;
            unlockedEvent.payload = achievement;
            m_eventBus.publish(unlockedEvent);
        }
    }

    AppEvent savedEvent;
    savedEvent.type = AppEventType::SessionSaved;
    savedEvent.payload = m_currentSession.task;
    m_eventBus.publish(savedEvent);

    emit sessionSaved(m_currentSession, achievements);

    m_hasActiveBreak = true;
    m_timer.startBreak(m_breakMinutes);

    AppEvent breakEvent;
    breakEvent.type = AppEventType::BreakStarted;
    breakEvent.payload = QString::number(m_breakMinutes);
    m_eventBus.publish(breakEvent);

    emit breakStarted(m_breakMinutes);
}

void FocusSessionManager::submitReviewAsync(const QStringList& completedGoals, const QStringList& problems, int selfRating) {
    if (m_aiBusy || m_currentSession.task.isEmpty()) {
        return;
    }

    FocusSession session = m_currentSession;
    session.completedGoals = completedGoals;
    session.problems = problems;
    session.problems.append(QString("自评分：%1/5").arg(selfRating));

    m_aiBusy = true;
    emit aiTaskStarted("Review");

    auto* watcher = new QFutureWatcher<FocusSession>(this);
    connect(watcher, &QFutureWatcher<FocusSession>::finished, this, [this, watcher]() {
        m_currentSession = watcher->result();
        m_currentSession.keyboardMouseActivityCount = m_inputActivityMonitor.activityCount();
        m_currentSession.maxIdleSeconds = m_inputActivityMonitor.maxIdleSeconds();
        watcher->deleteLater();
        m_aiBusy = false;

        const QStringList achievements = m_achievementSystem.update(m_currentSession, m_petProfile);
        m_storage.saveFocusSession(m_currentSession);
        m_storage.saveDistractionEvents(m_currentSession.id, m_focusMonitor.events());
        m_storage.savePetProfile(m_petProfile);
        for (const QString& achievement : achievements) {
            m_storage.saveAchievement(achievement);
        }

        AppEvent reviewEvent;
        reviewEvent.type = AppEventType::AiReviewGenerated;
        reviewEvent.payload = m_currentSession.aiSummary;
        m_eventBus.publish(reviewEvent);

        if (!achievements.isEmpty()) {
            AppEvent levelEvent;
            levelEvent.type = AppEventType::PetLevelUp;
            levelEvent.payload = achievements.join(", ");
            m_eventBus.publish(levelEvent);

            for (const QString& achievement : achievements) {
                AppEvent unlockedEvent;
                unlockedEvent.type = AppEventType::AchievementUnlocked;
                unlockedEvent.payload = achievement;
                m_eventBus.publish(unlockedEvent);
            }
        }

        AppEvent savedEvent;
        savedEvent.type = AppEventType::SessionSaved;
        savedEvent.payload = m_currentSession.task;
        m_eventBus.publish(savedEvent);

        emit sessionSaved(m_currentSession, achievements);
        emit aiTaskFinished("Review");

        m_hasActiveBreak = true;
        m_timer.startBreak(m_breakMinutes);

        AppEvent breakEvent;
        breakEvent.type = AppEventType::BreakStarted;
        breakEvent.payload = QString::number(m_breakMinutes);
        m_eventBus.publish(breakEvent);

        emit breakStarted(m_breakMinutes);
    });

    watcher->setFuture(QtConcurrent::run([this, session]() {
        return m_reviewGenerator.generateReview(session);
    }));
}

void FocusSessionManager::setBreakMinutes(int minutes) {
    m_breakMinutes = std::max(1, minutes);
}

FocusSession FocusSessionManager::currentSession() const {
    return m_currentSession;
}

PetProfile FocusSessionManager::petProfile() const {
    return m_petProfile;
}

void FocusSessionManager::onTimerCompleted() {
    if (m_hasActiveBreak) {
        m_hasActiveBreak = false;

        AppEvent event;
        event.type = AppEventType::BreakFinished;
        event.payload = "Break finished";
        m_eventBus.publish(event);

        emit breakFinished();
        return;
    }

    if (!m_hasActiveSession) {
        return;
    }

    m_focusMonitor.stopMonitoring();
    m_inputActivityMonitor.stopMonitoring();
    m_currentSession.actualMinutes = m_currentSession.plannedMinutes;
    m_currentSession.distractionCount = m_focusMonitor.events().size();
    m_currentSession.keyboardMouseActivityCount = m_inputActivityMonitor.activityCount();
    m_currentSession.maxIdleSeconds = m_inputActivityMonitor.maxIdleSeconds();
    m_hasActiveSession = false;

    AppEvent event;
    event.type = AppEventType::TimerCompleted;
    event.payload = m_currentSession.task;
    m_eventBus.publish(event);

    emit sessionCompleted(m_currentSession);
}

void FocusSessionManager::onAppEvent(const AppEvent& event) {
    if (event.type == AppEventType::DistractionDetected && m_hasActiveSession) {
        m_currentSession.distractionCount += 1;
        emit distractionCountChanged(m_currentSession.distractionCount);
    }
}
