#include "core/PetStateMachine.h"

PetStateMachine::PetStateMachine(EventBus& eventBus, QObject* parent)
    : QObject(parent), m_eventBus(eventBus) {
    connect(&m_eventBus, &EventBus::eventPublished, this, &PetStateMachine::handleEvent);
}

PetMood PetStateMachine::mood() const {
    return m_mood;
}

FocusState PetStateMachine::focusState() const {
    return m_focusState;
}

QString PetStateMachine::moodText() const {
    switch (m_mood) {
    case PetMood::Idle: return "Idle";
    case PetMood::Focused: return "Focused";
    case PetMood::Encouraging: return "Encouraging";
    case PetMood::Worried: return "Worried";
    case PetMood::Angry: return "Angry";
    case PetMood::Sleepy: return "Sleepy";
    case PetMood::Proud: return "Proud";
    case PetMood::Celebrating: return "Celebrating";
    }
    return "Unknown";
}

QString PetStateMachine::speechText() const {
    return m_speech;
}

void PetStateMachine::handleEvent(const AppEvent& event) {
    switch (event.type) {
    case AppEventType::TimerStarted:
        m_distractionCount = 0;
        setMood(PetMood::Focused, FocusState::Focusing, "I am here. Keep this step small and steady.");
        break;
    case AppEventType::TimerPaused:
        setMood(PetMood::Worried, FocusState::Distracted, "A pause is fine. Keep the task from scattering.");
        break;
    case AppEventType::TimerResumed:
        setMood(PetMood::Focused, FocusState::Focusing, "Nice return. Continue.");
        break;
    case AppEventType::TimerCompleted:
        setMood(PetMood::Celebrating, FocusState::Completed, "Round complete. Solid work.");
        break;
    case AppEventType::TimerCancelled:
        setMood(PetMood::Idle, FocusState::NotStarted, "Round cancelled. Start fresh next time.");
        break;
    case AppEventType::BreakStarted:
        setMood(PetMood::Sleepy, FocusState::BreakTime, "Take a short break and drink some water.");
        break;
    case AppEventType::BreakFinished:
        setMood(PetMood::Encouraging, FocusState::NotStarted, "Break finished. Ease into the next round.");
        break;
    case AppEventType::DistractionDetected:
        ++m_distractionCount;
        if (m_distractionCount >= 3) {
            setMood(PetMood::Angry, FocusState::Distracted, "Third drift detected. Return to the task window.");
        } else {
            setMood(PetMood::Worried, FocusState::Distracted, "Possible drift. Finish this small stretch first.");
        }
        break;
    case AppEventType::PetLevelUp:
        setMood(PetMood::Proud, m_focusState, "Level up. Consistency leaves a mark.");
        break;
    default:
        break;
    }
}

void PetStateMachine::setMood(PetMood mood, FocusState focusState, const QString& speech) {
    m_mood = mood;
    m_focusState = focusState;
    m_speech = speech;
    emit moodChanged(m_mood, m_speech);

    AppEvent event;
    event.type = AppEventType::PetMoodChanged;
    event.payload = moodText();
    m_eventBus.publish(event);
}
