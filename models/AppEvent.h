#pragma once

#include <QDateTime>
#include <QMetaType>
#include <QString>
#include <QVariantMap>

enum class AppEventType {
    TimerStarted,
    TimerPaused,
    TimerResumed,
    TimerCompleted,
    TimerCancelled,
    BreakStarted,
    BreakFinished,
    DistractionDetected,
    InputActivityDetected,
    InputIdleDetected,
    ReviewSubmitted,
    AiPlanGenerated,
    AiReviewGenerated,
    SessionSaved,
    PetMoodChanged,
    PetLevelUp,
    AchievementUnlocked
};

struct AppEvent {
    AppEventType type = AppEventType::TimerStarted;
    QString payload;
    QVariantMap data;
    QDateTime occurredAt = QDateTime::currentDateTime();
};

Q_DECLARE_METATYPE(AppEvent)
Q_DECLARE_METATYPE(AppEventType)
