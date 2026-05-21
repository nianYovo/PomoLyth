#pragma once

#include <QObject>
#include <QTimer>

enum class TimerState {
    Idle,
    Running,
    Paused,
    Completed,
    BreakTime,
    Cancelled
};

class PomodoroTimer : public QObject {
    Q_OBJECT

public:
    explicit PomodoroTimer(QObject* parent = nullptr);

    void start(int minutes);
    void pause();
    void resume();
    void stop();
    void startBreak(int minutes);

    int remainingSeconds() const;
    int totalSeconds() const;
    TimerState state() const;

signals:
    void ticked(int remainingSeconds, int totalSeconds);
    void stateChanged(TimerState state);
    void completed();

private slots:
    void onTick();

private:
    void setState(TimerState state);

    QTimer m_timer;
    int m_totalSeconds = 0;
    int m_remainingSeconds = 0;
    TimerState m_state = TimerState::Idle;
};
