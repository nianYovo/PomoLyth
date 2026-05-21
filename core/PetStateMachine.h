#pragma once

#include <QObject>
#include "core/EventBus.h"

enum class PetMood {
    Idle,
    Focused,
    Encouraging,
    Worried,
    Angry,
    Sleepy,
    Proud,
    Celebrating
};

enum class FocusState {
    NotStarted,
    Planning,
    Focusing,
    Distracted,
    BreakTime,
    Reviewing,
    Completed
};

class PetStateMachine : public QObject {
    Q_OBJECT

public:
    explicit PetStateMachine(EventBus& eventBus, QObject* parent = nullptr);

    PetMood mood() const;
    FocusState focusState() const;
    QString moodText() const;
    QString speechText() const;

signals:
    void moodChanged(PetMood mood, const QString& speech);

private:
    void handleEvent(const AppEvent& event);
    void setMood(PetMood mood, FocusState focusState, const QString& speech);

    EventBus& m_eventBus;
    PetMood m_mood = PetMood::Idle;
    FocusState m_focusState = FocusState::NotStarted;
    QString m_speech = "准备好开始一轮专注了吗？";
    int m_distractionCount = 0;
};
