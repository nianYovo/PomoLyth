#include "ui/MainWindow.h"

#include <QApplication>
#include <QCloseEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSplitter>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include "ui/DashboardWindow.h"
#include "ui/PetWidget.h"
#include "ui/PetWindow.h"
#include "ui/ReviewDialog.h"
#include "ui/SettingsDialog.h"
#include "ui/TimerPanel.h"

MainWindow::MainWindow(
    FocusSessionManager& sessionManager,
    PomodoroTimer& timer,
    PetStateMachine& petStateMachine,
    SQLiteStorage& storage,
    JsonStorage& jsonStorage,
    FocusMonitor& focusMonitor,
    InputActivityMonitor& inputActivityMonitor,
    ReviewGenerator& reviewGenerator,
    const AppConfig& config,
    QWidget* parent)
    : QMainWindow(parent),
      m_sessionManager(sessionManager),
      m_timer(timer),
      m_petStateMachine(petStateMachine),
      m_storage(storage),
      m_jsonStorage(jsonStorage),
      m_focusMonitor(focusMonitor),
      m_inputActivityMonitor(inputActivityMonitor),
      m_reviewGenerator(reviewGenerator),
      m_config(config),
      m_blacklist(jsonStorage.loadBlacklist()),
      m_whitelist(jsonStorage.loadWhitelist()) {
    buildUi();
    applyStyle();
    m_timerPanel->setDefaultMinutes(m_config.defaultFocusMinutes);

    connect(m_timerPanel, &TimerPanel::planRequested, this, &MainWindow::requestPlan);
    connect(m_timerPanel, &TimerPanel::startRequested, this, &MainWindow::startSession);
    connect(m_timerPanel, &TimerPanel::pauseRequested, &m_sessionManager, &FocusSessionManager::pauseSession);
    connect(m_timerPanel, &TimerPanel::resumeRequested, &m_sessionManager, &FocusSessionManager::resumeSession);
    connect(m_timerPanel, &TimerPanel::stopRequested, this, &MainWindow::cancelSessionWithConfirm);
    connect(m_timerPanel, &TimerPanel::dashboardRequested, this, &MainWindow::showDashboard);
    connect(m_timerPanel, &TimerPanel::petWindowRequested, this, &MainWindow::togglePetWindow);
    connect(m_timerPanel, &TimerPanel::settingsRequested, this, &MainWindow::showSettings);

    connect(&m_timer, &PomodoroTimer::ticked, m_timerPanel, &TimerPanel::setTime);
    connect(&m_timer, &PomodoroTimer::stateChanged, this, [this](TimerState state) {
        m_timerPanel->setRunning(state == TimerState::Running);
        m_timerPanel->setPaused(state == TimerState::Paused);
        if (state == TimerState::Cancelled || state == TimerState::Completed) {
            m_timerPanel->setRunning(false);
        }
    });

    connect(&m_sessionManager, &FocusSessionManager::planGenerated, this, &MainWindow::updatePlanView);
    connect(&m_sessionManager, &FocusSessionManager::aiTaskStarted, this, [this](const QString& label) {
        m_statusLabel->setText(QString("%1 正在生成...").arg(label));
    });
    connect(&m_sessionManager, &FocusSessionManager::aiTaskFinished, this, [this](const QString& label) {
        m_statusLabel->setText(QString("%1 已完成。").arg(label));
    });
    connect(&m_sessionManager, &FocusSessionManager::sessionCompleted, this, &MainWindow::showReview);
    connect(&m_sessionManager, &FocusSessionManager::sessionSaved, this, [this](const FocusSession& session, const QStringList& achievements) {
        QString message = QString("已保存：%1\nAI 复盘：%2").arg(session.task, session.aiSummary);
        if (!achievements.isEmpty()) {
            message += QString("\n解锁成就：%1").arg(achievements.join("、"));
        }
        m_statusLabel->setText(message);
        updatePetProfileView();
        QMessageBox::information(this, "本轮复盘", message);
        if (m_dashboard) {
            m_dashboard->refresh();
        }
    });
    connect(&m_sessionManager, &FocusSessionManager::distractionCountChanged, this, [this](int count) {
        m_statusLabel->setText(QString("检测到分心次数：%1").arg(count));
    });
    connect(&m_sessionManager, &FocusSessionManager::breakStarted, this, [this](int minutes) {
        m_statusLabel->setText(QString("已进入 %1 分钟短休息。").arg(minutes));
    });
    connect(&m_sessionManager, &FocusSessionManager::breakFinished, this, [this]() {
        m_statusLabel->setText("短休息结束，可以准备下一轮。");
    });
    connect(&m_inputActivityMonitor, &InputActivityMonitor::activityChanged, this, [this](int count, int idleSeconds) {
        m_statusLabel->setText(QString("键鼠活动：%1 次，当前空闲：%2 秒").arg(count).arg(idleSeconds));
    });
    connect(&m_inputActivityMonitor, &InputActivityMonitor::idleWarning, this, [this](int idleSeconds) {
        m_statusLabel->setText(QString("已经 %1 秒没有键盘或鼠标输入，还在专注吗？").arg(idleSeconds));
    });
    connect(&m_petStateMachine, &PetStateMachine::moodChanged, m_petWidget, &PetWidget::setMood);

    QTimer::singleShot(300, this, [this]() {
        m_statusLabel->setText(m_statusLabel->text() + " 点击“悬浮桌宠”可显示透明桌面宠物。");
    });
}

void MainWindow::buildUi() {
    setWindowTitle("PomoLyth");
    resize(1100, 680);

    m_taskInput = new QLineEdit;
    m_taskInput->setPlaceholderText("输入本轮任务，例如：复习 C++ STL 的 vector 和 map");

    m_planView = new QTextEdit;
    m_planView->setReadOnly(true);
    m_planView->setPlaceholderText("AI 任务计划会显示在这里。");

    m_statusLabel = new QLabel(QString("准备开始一轮专注。AI Provider：%1").arg(m_config.aiProvider));
    m_statusLabel->setWordWrap(true);
    m_petProfileLabel = new QLabel;
    m_petProfileLabel->setObjectName("petProfileLabel");
    m_petProfileLabel->setWordWrap(true);

    m_timerPanel = new TimerPanel;
    m_petWidget = new PetWidget;
    m_petWidget->setMood(m_petStateMachine.mood(), m_petStateMachine.speechText());

    auto* title = new QLabel("PomoLyth");
    title->setObjectName("appTitle");
    auto* subtitle = new QLabel("和桌宠一起规划、专注、复盘、成长。");
    subtitle->setObjectName("appSubtitle");

    auto* taskTitle = new QLabel("当前任务");
    taskTitle->setObjectName("sectionTitle");
    auto* planTitle = new QLabel("专注计划");
    planTitle->setObjectName("sectionTitle");
    auto* petTitle = new QLabel("桌宠陪伴");
    petTitle->setObjectName("sectionTitle");
    auto* growthTitle = new QLabel("成长状态");
    growthTitle->setObjectName("sectionTitle");

    auto* left = new QFrame;
    left->setObjectName("panel");
    auto* leftLayout = new QVBoxLayout(left);
    leftLayout->setContentsMargins(22, 22, 22, 22);
    leftLayout->setSpacing(14);
    leftLayout->addWidget(title);
    leftLayout->addWidget(subtitle);
    leftLayout->addSpacing(8);
    leftLayout->addWidget(taskTitle);
    leftLayout->addWidget(m_taskInput);
    leftLayout->addWidget(m_timerPanel);
    leftLayout->addWidget(planTitle);
    leftLayout->addWidget(m_planView, 1);
    leftLayout->addWidget(m_statusLabel);

    auto* right = new QFrame;
    right->setObjectName("sidePanel");
    auto* rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(22, 22, 22, 22);
    rightLayout->setSpacing(14);
    rightLayout->addWidget(petTitle);
    rightLayout->addWidget(m_petWidget, 1);
    rightLayout->addWidget(growthTitle);
    rightLayout->addWidget(m_petProfileLabel);
    rightLayout->addStretch();

    auto* root = new QWidget;
    root->setObjectName("root");
    auto* rootLayout = new QHBoxLayout(root);
    rootLayout->setContentsMargins(20, 20, 20, 20);
    rootLayout->setSpacing(18);
    rootLayout->addWidget(left, 7);
    rootLayout->addWidget(right, 3);

    setCentralWidget(root);
    updatePetProfileView();
}

void MainWindow::applyStyle() {
    qApp->setStyleSheet(
        "QMainWindow, #root { background: #eef3ea; color: #25302a; font-size: 14px; }"
        "#panel, #sidePanel { background: #fbfcf7; border: 1px solid #d8e0d4; border-radius: 10px; }"
        "#sidePanel { background: #f7faf4; }"
        "#appTitle { font-size: 34px; font-weight: 800; color: #1f3d31; }"
        "#appSubtitle { color: #667565; font-size: 14px; }"
        "#sectionTitle { color: #345347; font-weight: 700; font-size: 15px; padding-top: 4px; }"
        "QLineEdit, QTextEdit, QSpinBox, QTableWidget { background: #ffffff; border: 1px solid #cfd8cc; border-radius: 8px; padding: 9px; selection-background-color: #b8d8c8; }"
        "QLineEdit:focus, QTextEdit:focus, QSpinBox:focus { border: 1px solid #4f8068; }"
        "QPushButton { background: #335c4b; color: white; border: 0; border-radius: 8px; padding: 9px 13px; font-weight: 600; }"
        "QPushButton:disabled { background: #aeb8ad; }"
        "QPushButton:hover { background: #274739; }"
        "QPushButton:pressed { background: #1f362c; }"
        "#timeLabel { font-size: 54px; font-weight: 700; color: #20352b; }"
        "#metricLabel, #petProfileLabel, #dailyReportLabel { background: #ffffff; border: 1px solid #d6ddd3; border-radius: 8px; padding: 12px; font-size: 14px; }"
        "QSplitter::handle { background: transparent; }");
}

void MainWindow::requestPlan() {
    const QString input = m_taskInput->text().trimmed();
    if (input.isEmpty()) {
        QMessageBox::information(this, "需要任务", "请先输入本轮要专注完成的任务。");
        return;
    }
    m_sessionManager.requestPlan(input, m_timerPanel->minutes());
}

void MainWindow::startSession() {
    const QString input = m_taskInput->text().trimmed();
    if (input.isEmpty()) {
        QMessageBox::information(this, "需要任务", "请先输入本轮要专注完成的任务。");
        return;
    }
    if (m_currentTask.originalInput != input) {
        m_sessionManager.requestPlan(input, m_timerPanel->minutes());
        m_statusLabel->setText("正在先生成计划。计划出现后请再次点击开始。");
        return;
    }
    m_sessionManager.startSession(m_currentTask);
}

void MainWindow::cancelSessionWithConfirm() {
    const QMessageBox::StandardButton result = QMessageBox::question(
        this,
        "结束本轮",
        "确定要结束当前计时吗？这轮不会保存为完成记录。");
    if (result == QMessageBox::Yes) {
        m_sessionManager.cancelSession();
        m_statusLabel->setText("当前计时已结束。");
    }
}

void MainWindow::showDashboard() {
    if (!m_dashboard) {
        m_dashboard = new DashboardWindow(m_storage, m_reviewGenerator);
    }
    m_dashboard->refresh();
    m_dashboard->show();
    m_dashboard->raise();
    m_dashboard->activateWindow();
}

void MainWindow::togglePetWindow() {
    if (!m_petWindow) {
        m_petWindow = new PetWindow;
        m_petWindow->setMood(m_petStateMachine.mood(), m_petStateMachine.speechText());
        connect(&m_petStateMachine, &PetStateMachine::moodChanged, m_petWindow, &PetWindow::setMood);
    }

    if (m_petWindow->isVisible()) {
        m_petWindow->hide();
    } else {
        m_petWindow->show();
        m_petWindow->raise();
    }
}

void MainWindow::showSettings() {
    SettingsDialog dialog(m_config, m_blacklist, m_whitelist, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    m_config = dialog.appConfig();
    m_blacklist = dialog.blacklist();
    m_whitelist = dialog.whitelist();

    const bool saved =
        m_jsonStorage.saveAppConfig(m_config) &&
        m_jsonStorage.saveBlacklist(m_blacklist) &&
        m_jsonStorage.saveWhitelist(m_whitelist);

    m_timerPanel->setDefaultMinutes(m_config.defaultFocusMinutes);
    m_sessionManager.setBreakMinutes(m_config.defaultBreakMinutes);
    m_focusMonitor.setIntervalSeconds(m_config.focusMonitorIntervalSeconds);
    m_focusMonitor.setBlacklist(m_blacklist);
    m_focusMonitor.setWhitelist(m_whitelist);

    m_statusLabel->setText(saved ? "设置已保存并应用。" : "设置已应用，但写入配置文件失败。");
}

void MainWindow::showReview(const FocusSession& session) {
    Q_UNUSED(session)
    ReviewDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        m_sessionManager.submitReviewAsync(dialog.completedGoals(), dialog.problems(), dialog.selfRating());
    }
}

void MainWindow::updatePlanView(const FocusTask& task) {
    m_currentTask = task;
    QString text;
    text += QString("任务：%1\n").arg(task.originalInput);
    text += QString("预计：%1 分钟\n").arg(task.estimatedMinutes);
    text += QString("难度：%1\n\n").arg(task.difficulty);
    text += "本轮目标：\n";
    for (const QString& goal : task.goals) {
        text += QString("- %1\n").arg(goal);
    }
    text += "\n本轮避免：\n";
    for (const QString& item : task.avoidList) {
        text += QString("- %1\n").arg(item);
    }
    m_planView->setPlainText(text);
    m_statusLabel->setText("计划已生成，可以开始专注。");
}

void MainWindow::updatePetProfileView() {
    const PetProfile profile = m_sessionManager.petProfile();
    m_petProfileLabel->setText(QString("桌宠 Lv.%1  EXP %2/100  亲密度 %3  累计专注 %4 分钟  番茄轮数 %5")
        .arg(profile.level)
        .arg(profile.exp % 100)
        .arg(profile.intimacy)
        .arg(profile.totalFocusMinutes)
        .arg(profile.completedPomodoros));
}

void MainWindow::closeEvent(QCloseEvent* event) {
    hide();
    event->ignore();
}
