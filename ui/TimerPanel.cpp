#include "ui/TimerPanel.h"

#include <algorithm>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QSpinBox>
#include <QVBoxLayout>

TimerPanel::TimerPanel(QWidget* parent) : QWidget(parent) {
    setObjectName("timerCard");
    setAttribute(Qt::WA_StyledBackground, true);

    m_timeLabel = new QLabel("25:00");
    m_timeLabel->setAlignment(Qt::AlignCenter);
    m_timeLabel->setObjectName("timeLabel");

    m_stateLabel = new QLabel("准备就绪");
    m_stateLabel->setAlignment(Qt::AlignCenter);
    m_stateLabel->setObjectName("statePill");

    m_minutesSpin = new QSpinBox;
    m_minutesSpin->setRange(1, 180);
    m_minutesSpin->setValue(25);
    m_minutesSpin->setSuffix(" min");
    m_minutesSpin->setAccelerated(true);
    m_minutesSpin->setObjectName("durationSpin");
    m_minutesSpin->setFixedWidth(128);
    m_minutesSpin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_planButton = new QPushButton("生成计划");
    m_planButton->setParent(this);
    m_planButton->hide();
    m_startButton = new QPushButton("开始");
    m_pauseButton = new QPushButton("暂停");
    m_resumeButton = new QPushButton("继续");
    m_stopButton = new QPushButton("结束");
    m_dashboardButton = new QPushButton("数据面板");
    m_petWindowButton = new QPushButton("悬浮桌宠");
    m_settingsButton = new QPushButton("设置");

    m_planButton->setProperty("buttonRole", "mint");
    m_startButton->setProperty("buttonRole", "primary");
    m_pauseButton->setProperty("buttonRole", "outline");
    m_resumeButton->setProperty("buttonRole", "outline");
    m_stopButton->setProperty("buttonRole", "outline");
    m_petWindowButton->setProperty("buttonRole", "outline");
    m_dashboardButton->setProperty("buttonRole", "outline");
    m_settingsButton->setProperty("buttonRole", "outline");

    const QList<QPushButton*> buttons = {
        m_planButton,
        m_startButton,
        m_pauseButton,
        m_resumeButton,
        m_stopButton,
        m_petWindowButton,
        m_dashboardButton,
        m_settingsButton,
    };
    for (auto* button : buttons) {
        button->setCursor(Qt::PointingHandCursor);
        button->setMinimumHeight(40);
    }
    m_startButton->setMinimumWidth(138);
    m_pauseButton->setMinimumWidth(138);
    m_resumeButton->setMinimumWidth(138);
    m_stopButton->setMinimumWidth(138);
    m_petWindowButton->setMinimumWidth(98);
    m_dashboardButton->setMinimumWidth(98);
    m_settingsButton->setMinimumWidth(88);

    auto* durationLabel = new QLabel("专注时长");
    durationLabel->setObjectName("controlLabel");

    auto* durationLayout = new QHBoxLayout;
    durationLayout->setContentsMargins(0, 0, 0, 0);
    durationLayout->setSpacing(10);
    durationLayout->addStretch();
    durationLayout->addWidget(durationLabel);
    durationLayout->addWidget(m_minutesSpin);
    durationLayout->addStretch();

    auto* buttonLayout = new QGridLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setHorizontalSpacing(10);
    buttonLayout->setVerticalSpacing(10);
    buttonLayout->addWidget(m_startButton, 0, 0);
    buttonLayout->addWidget(m_pauseButton, 0, 1);
    buttonLayout->addWidget(m_resumeButton, 1, 0);
    buttonLayout->addWidget(m_stopButton, 1, 1);
    buttonLayout->setColumnStretch(0, 1);
    buttonLayout->setColumnStretch(1, 1);

    auto* toolsLayout = new QHBoxLayout;
    toolsLayout->setContentsMargins(0, 0, 0, 0);
    toolsLayout->setSpacing(10);
    toolsLayout->addStretch();
    toolsLayout->addWidget(m_petWindowButton);
    toolsLayout->addWidget(m_dashboardButton);
    toolsLayout->addWidget(m_settingsButton);
    toolsLayout->addStretch();

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(22, 16, 22, 16);
    layout->setSpacing(12);
    layout->addWidget(m_timeLabel);
    layout->addWidget(m_stateLabel, 0, Qt::AlignHCenter);
    layout->addLayout(durationLayout);
    layout->addLayout(buttonLayout);
    layout->addLayout(toolsLayout);

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
