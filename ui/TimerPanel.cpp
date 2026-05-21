#include "ui/TimerPanel.h"

#include <algorithm>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

TimerPanel::TimerPanel(QWidget* parent) : QWidget(parent) {
    m_timeLabel = new QLabel("25:00");
    m_timeLabel->setAlignment(Qt::AlignCenter);
    m_timeLabel->setObjectName("timeLabel");

    m_stateLabel = new QLabel("准备就绪");
    m_stateLabel->setAlignment(Qt::AlignCenter);

    m_minutesSpin = new QSpinBox;
    m_minutesSpin->setRange(1, 180);
    m_minutesSpin->setValue(25);
    m_minutesSpin->setSuffix(" min");

    m_planButton = new QPushButton("生成计划");
    m_startButton = new QPushButton("开始");
    m_pauseButton = new QPushButton("暂停");
    m_resumeButton = new QPushButton("继续");
    m_stopButton = new QPushButton("结束");
    m_dashboardButton = new QPushButton("数据面板");
    m_petWindowButton = new QPushButton("悬浮桌宠");
    m_settingsButton = new QPushButton("设置");

    auto* durationLayout = new QHBoxLayout;
    durationLayout->addWidget(new QLabel("时长"));
    durationLayout->addWidget(m_minutesSpin);
    durationLayout->addStretch();
    durationLayout->addWidget(m_petWindowButton);
    durationLayout->addWidget(m_dashboardButton);
    durationLayout->addWidget(m_settingsButton);

    auto* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_planButton);
    buttonLayout->addWidget(m_startButton);
    buttonLayout->addWidget(m_pauseButton);
    buttonLayout->addWidget(m_resumeButton);
    buttonLayout->addWidget(m_stopButton);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(m_timeLabel);
    layout->addWidget(m_stateLabel);
    layout->addLayout(durationLayout);
    layout->addLayout(buttonLayout);

    connect(m_planButton, &QPushButton::clicked, this, &TimerPanel::planRequested);
    connect(m_startButton, &QPushButton::clicked, this, &TimerPanel::startRequested);
    connect(m_pauseButton, &QPushButton::clicked, this, &TimerPanel::pauseRequested);
    connect(m_resumeButton, &QPushButton::clicked, this, &TimerPanel::resumeRequested);
    connect(m_stopButton, &QPushButton::clicked, this, &TimerPanel::stopRequested);
    connect(m_dashboardButton, &QPushButton::clicked, this, &TimerPanel::dashboardRequested);
    connect(m_petWindowButton, &QPushButton::clicked, this, &TimerPanel::petWindowRequested);
    connect(m_settingsButton, &QPushButton::clicked, this, &TimerPanel::settingsRequested);

    setRunning(false);
    setPaused(false);
}

int TimerPanel::minutes() const {
    return m_minutesSpin->value();
}

void TimerPanel::setDefaultMinutes(int minutes) {
    m_minutesSpin->setValue(std::clamp(minutes, m_minutesSpin->minimum(), m_minutesSpin->maximum()));
    setTime(m_minutesSpin->value() * 60, m_minutesSpin->value() * 60);
}

void TimerPanel::setTime(int remainingSeconds, int totalSeconds) {
    Q_UNUSED(totalSeconds)
    const int minutes = remainingSeconds / 60;
    const int seconds = remainingSeconds % 60;
    m_timeLabel->setText(QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0')));
}

void TimerPanel::setRunning(bool running) {
    m_stateLabel->setText(running ? "专注中" : "准备就绪");
    m_startButton->setEnabled(!running);
    m_pauseButton->setEnabled(running);
    m_stopButton->setEnabled(running);
    m_minutesSpin->setEnabled(!running);
}

void TimerPanel::setPaused(bool paused) {
    m_stateLabel->setText(paused ? "已暂停" : m_stateLabel->text());
    m_pauseButton->setEnabled(!paused);
    m_resumeButton->setEnabled(paused);
}
