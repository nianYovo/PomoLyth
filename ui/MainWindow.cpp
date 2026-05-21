#include "ui/MainWindow.h"

#include <QApplication>
#include <QCloseEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSplitter>
#include <QTextEdit>
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
        m_statusLabel->setText(QString("%1 is running...").arg(label));
    });
    connect(&m_sessionManager, &FocusSessionManager::aiTaskFinished, this, [this](const QString& label) {
        m_statusLabel->setText(QString("%1 finished.").arg(label));
    });
    connect(&m_sessionManager, &FocusSessionManager::sessionCompleted, this, &MainWindow::showReview);
    connect(&m_sessionManager, &FocusSessionManager::sessionSaved, this, [this](const FocusSession& session, const QStringList& achievements) {
        QString message = QString("Saved: %1\nAI review: %2").arg(session.task, session.aiSummary);
        if (!achievements.isEmpty()) {
            message += QString("\nUnlocked: %1").arg(achievements.join(", "));
        }
        m_statusLabel->setText(message);
        updatePetProfileView();
        QMessageBox::information(this, "Session Review", message);
        if (m_dashboard) {
            m_dashboard->refresh();
        }
    });
    connect(&m_sessionManager, &FocusSessionManager::distractionCountChanged, this, [this](int count) {
        m_statusLabel->setText(QString("Distractions detected: %1").arg(count));
    });
    connect(&m_sessionManager, &FocusSessionManager::breakStarted, this, [this](int minutes) {
        m_statusLabel->setText(QString("Break started: %1 minutes.").arg(minutes));
    });
    connect(&m_sessionManager, &FocusSessionManager::breakFinished, this, [this]() {
        m_statusLabel->setText("Break finished. Ready for the next round.");
    });
    connect(&m_petStateMachine, &PetStateMachine::moodChanged, m_petWidget, &PetWidget::setMood);
}

void MainWindow::buildUi() {
    setWindowTitle("PomoLyth");
    resize(980, 640);

    m_taskInput = new QLineEdit;
    m_taskInput->setPlaceholderText("Enter this round's task, e.g. review C++ STL vector and map");

    m_planView = new QTextEdit;
    m_planView->setReadOnly(true);
    m_planView->setPlaceholderText("The AI task plan will appear here.");

    m_statusLabel = new QLabel(QString("Ready for a focus round? AI provider: %1").arg(m_config.aiProvider));
    m_statusLabel->setWordWrap(true);
    m_petProfileLabel = new QLabel;
    m_petProfileLabel->setObjectName("petProfileLabel");
    m_petProfileLabel->setWordWrap(true);

    m_timerPanel = new TimerPanel;
    m_petWidget = new PetWidget;
    m_petWidget->setMood(m_petStateMachine.mood(), m_petStateMachine.speechText());

    auto* left = new QWidget;
    auto* leftLayout = new QVBoxLayout(left);
    leftLayout->addWidget(new QLabel("Current task"));
    leftLayout->addWidget(m_taskInput);
    leftLayout->addWidget(m_timerPanel);
    leftLayout->addWidget(new QLabel("Focus plan"));
    leftLayout->addWidget(m_planView, 1);
    leftLayout->addWidget(m_petProfileLabel);
    leftLayout->addWidget(m_statusLabel);

    auto* splitter = new QSplitter;
    splitter->addWidget(left);
    splitter->addWidget(m_petWidget);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    setCentralWidget(splitter);
    updatePetProfileView();
}

void MainWindow::applyStyle() {
    qApp->setStyleSheet(
        "QMainWindow, QWidget { background: #f7f8f3; color: #25302a; font-size: 14px; }"
        "QLineEdit, QTextEdit, QSpinBox, QTableWidget { background: #ffffff; border: 1px solid #cfd8cc; border-radius: 6px; padding: 8px; }"
        "QPushButton { background: #335c4b; color: white; border: 0; border-radius: 6px; padding: 8px 12px; }"
        "QPushButton:disabled { background: #aeb8ad; }"
        "QPushButton:hover { background: #274739; }"
        "#timeLabel { font-size: 54px; font-weight: 700; color: #20352b; }"
        "#metricLabel, #petProfileLabel, #dailyReportLabel { background: #ffffff; border: 1px solid #d6ddd3; border-radius: 8px; padding: 10px; font-size: 14px; }");
}

void MainWindow::requestPlan() {
    const QString input = m_taskInput->text().trimmed();
    if (input.isEmpty()) {
        QMessageBox::information(this, "Task required", "Please enter what you want to focus on.");
        return;
    }
    m_sessionManager.requestPlan(input, m_timerPanel->minutes());
}

void MainWindow::startSession() {
    const QString input = m_taskInput->text().trimmed();
    if (input.isEmpty()) {
        QMessageBox::information(this, "Task required", "Please enter what you want to focus on.");
        return;
    }
    if (m_currentTask.originalInput != input) {
        m_sessionManager.requestPlan(input, m_timerPanel->minutes());
        m_statusLabel->setText("Planning first. Start again after the plan appears.");
        return;
    }
    m_sessionManager.startSession(m_currentTask);
}

void MainWindow::cancelSessionWithConfirm() {
    const QMessageBox::StandardButton result = QMessageBox::question(
        this,
        "Stop round",
        "Stop the current timer? This round will not be saved as completed.");
    if (result == QMessageBox::Yes) {
        m_sessionManager.cancelSession();
        m_statusLabel->setText("Current timer stopped.");
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

    m_statusLabel->setText(saved ? "Settings saved and applied." : "Settings applied, but saving config files failed.");
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
    text += QString("Task: %1\n").arg(task.originalInput);
    text += QString("Estimate: %1 minutes\n").arg(task.estimatedMinutes);
    text += QString("Difficulty: %1\n\n").arg(task.difficulty);
    text += "Goals:\n";
    for (const QString& goal : task.goals) {
        text += QString("- %1\n").arg(goal);
    }
    text += "\nAvoid:\n";
    for (const QString& item : task.avoidList) {
        text += QString("- %1\n").arg(item);
    }
    m_planView->setPlainText(text);
    m_statusLabel->setText("Plan generated. You can start focusing.");
}

void MainWindow::updatePetProfileView() {
    const PetProfile profile = m_sessionManager.petProfile();
    m_petProfileLabel->setText(QString("Pet Lv.%1  EXP %2/100  Intimacy %3  Focus %4 min  Rounds %5")
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
