#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class QSpinBox;

class TimerPanel : public QWidget {
    Q_OBJECT

public:
    explicit TimerPanel(QWidget* parent = nullptr);

    int minutes() const;
    void setDefaultMinutes(int minutes);
    void setTime(int remainingSeconds, int totalSeconds);
    void setRunning(bool running);
    void setPaused(bool paused);

signals:
    void planRequested();
    void startRequested();
    void pauseRequested();
    void resumeRequested();
    void stopRequested();
    void dashboardRequested();
    void petWindowRequested();
    void settingsRequested();

private:
    QLabel* m_timeLabel = nullptr;
    QLabel* m_stateLabel = nullptr;
    QSpinBox* m_minutesSpin = nullptr;
    QPushButton* m_planButton = nullptr;
    QPushButton* m_startButton = nullptr;
    QPushButton* m_pauseButton = nullptr;
    QPushButton* m_resumeButton = nullptr;
    QPushButton* m_stopButton = nullptr;
    QPushButton* m_dashboardButton = nullptr;
    QPushButton* m_petWindowButton = nullptr;
    QPushButton* m_settingsButton = nullptr;
};
