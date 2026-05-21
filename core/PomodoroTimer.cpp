#include "core/PomodoroTimer.h"

#include <algorithm>

PomodoroTimer::PomodoroTimer(QObject* parent) : QObject(parent) {
    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, this, &PomodoroTimer::onTick);
}

void PomodoroTimer::start(int minutes) {
    m_totalSeconds = std::max(1, minutes) * 60;
    m_remainingSeconds = m_totalSeconds;
    setState(TimerState::Running);
    emit ticked(m_remainingSeconds, m_totalSeconds);
    m_timer.start();
}

void PomodoroTimer::pause() {
    if (m_state != TimerState::Running) {
        return;
    }
    m_timer.stop();
    setState(TimerState::Paused);
}

void PomodoroTimer::resume() {
    if (m_state != TimerState::Paused) {
        return;
    }
    setState(TimerState::Running);
    m_timer.start();
}

void PomodoroTimer::stop() {
    m_timer.stop();
    m_remainingSeconds = 0;
    setState(TimerState::Cancelled);
    emit ticked(m_remainingSeconds, m_totalSeconds);
}

void PomodoroTimer::startBreak(int minutes) {
    m_totalSeconds = std::max(1, minutes) * 60;
    m_remainingSeconds = m_totalSeconds;
    setState(TimerState::BreakTime);
    emit ticked(m_remainingSeconds, m_totalSeconds);
    m_timer.start();
}

int PomodoroTimer::remainingSeconds() const {
    return m_remainingSeconds;
}

int PomodoroTimer::totalSeconds() const {
    return m_totalSeconds;
}

TimerState PomodoroTimer::state() const {
    return m_state;
}

void PomodoroTimer::onTick() {
    if (m_remainingSeconds > 0) {
        --m_remainingSeconds;
        emit ticked(m_remainingSeconds, m_totalSeconds);
    }

    if (m_remainingSeconds <= 0) {
        m_timer.stop();
        setState(TimerState::Completed);
        emit completed();
    }
}

void PomodoroTimer::setState(TimerState state) {
    if (m_state == state) {
        return;
    }
    m_state = state;
    emit stateChanged(m_state);
}
