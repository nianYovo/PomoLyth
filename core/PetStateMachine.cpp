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
        setMood(PetMood::Focused, FocusState::Focusing, "我在旁边陪你，先把这一小步做好。");
        break;
    case AppEventType::TimerPaused:
        setMood(PetMood::Worried, FocusState::Distracted, "暂停一下没关系，别让任务散掉。");
        break;
    case AppEventType::TimerResumed:
        setMood(PetMood::Focused, FocusState::Focusing, "回来得很快，继续。");
        break;
    case AppEventType::TimerCompleted:
        setMood(PetMood::Celebrating, FocusState::Completed, "完成一轮！这一下很扎实。");
        break;
    case AppEventType::TimerCancelled:
        setMood(PetMood::Idle, FocusState::NotStarted, "这轮先收起，下一轮重新来。");
        break;
    case AppEventType::BreakStarted:
        setMood(PetMood::Sleepy, FocusState::BreakTime, "休息一下，喝口水。");
        break;
    case AppEventType::BreakFinished:
        setMood(PetMood::Encouraging, FocusState::NotStarted, "休息结束，下一轮可以慢慢启动。");
        break;
    case AppEventType::DistractionDetected:
        ++m_distractionCount;
        if (m_distractionCount >= 3) {
            setMood(PetMood::Angry, FocusState::Distracted, "这已经是第三次偏航了，先回到任务窗口。");
        } else {
            setMood(PetMood::Worried, FocusState::Distracted, "你是不是跑偏啦？先撑完这一小段。");
        }
        break;
    case AppEventType::PetLevelUp:
        setMood(PetMood::Proud, m_focusState, "升级了！长期坚持真的会留下痕迹。");
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
