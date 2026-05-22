#include "ui/MainWindow.h"

#include <QApplication>
#include <QCloseEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSizePolicy>
#include <QSplitter>
#include <QTextEdit>
#include <QTextDocument>
#include <QTimer>
#include <QVBoxLayout>
#include "ui/DashboardWindow.h"
#include "ui/PetWidget.h"
#include "ui/PetWindow.h"
#include "ui/ReviewDialog.h"
#include "ui/SettingsDialog.h"
#include "ui/TimerPanel.h"

static void addSoftShadow(QWidget* widget, int blurRadius = 24) {
    auto* shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(blurRadius);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(255, 141, 179, 42));
    widget->setGraphicsEffect(shadow);
}

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
    connect(&m_petStateMachine, &PetStateMachine::moodChanged, m_petWidget, &PetWidget::setMood);

    QTimer::singleShot(300, this, [this]() {
        m_statusLabel->setText(m_statusLabel->text() + " 点击“悬浮桌宠”可显示透明桌面宠物。");
    });
}

void MainWindow::buildUi() {
    setWindowTitle("PomoLyth");
    resize(1180, 700);
    setMinimumSize(1100, 660);

    m_taskInput = new QLineEdit;
    m_taskInput->setObjectName("taskInput");
    m_taskInput->setPlaceholderText("写下这一轮的小目标，比如：复习 vector 和 map 🍓");
    m_taskInput->setMaximumWidth(520);
    m_taskInput->setMinimumHeight(42);
    m_taskInput->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    m_planView = new QTextEdit;
    m_planView->setReadOnly(true);
    m_planView->setAcceptRichText(true);
    m_planView->setMinimumHeight(250);
    m_planView->setMaximumHeight(360);
    m_planView->setObjectName("planView");
    m_planView->document()->setDocumentMargin(14);
    m_planView->setPlaceholderText("生成计划后，桌宠会把任务拆成温柔的小步骤。");

    m_statusLabel = new QLabel(QString("准备开始一轮专注。AI Provider：%1").arg(m_config.aiProvider));
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setMinimumHeight(48);
    m_petProfileLabel = new QLabel;
    m_petProfileLabel->setObjectName("petProfileLabel");
    m_petProfileLabel->setWordWrap(true);
    m_expProgress = new QProgressBar;
    m_expProgress->setObjectName("expProgress");
    m_expProgress->setRange(0, 100);
    m_expProgress->setTextVisible(false);

    m_timerPanel = new TimerPanel;
    m_timerPanel->setFixedSize(380, 350);
    m_timerPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_petWidget = new PetWidget;
    m_petWidget->setMaximumWidth(270);
    m_petWidget->setMood(m_petStateMachine.mood(), m_petStateMachine.speechText());

    auto* taskPlanButton = new QPushButton("生成计划");
    taskPlanButton->setProperty("buttonRole", "mint");
    taskPlanButton->setMinimumHeight(42);
    taskPlanButton->setFixedWidth(118);
    taskPlanButton->setCursor(Qt::PointingHandCursor);
    connect(taskPlanButton, &QPushButton::clicked, this, &MainWindow::requestPlan);

    auto* title = new QLabel("PomoLyth");
    title->setObjectName("appTitle");
    auto* subtitle = new QLabel("和桌宠一起，把今天慢慢完成。");
    subtitle->setObjectName("appSubtitle");

    auto* taskTitle = new QLabel("今日小目标");
    taskTitle->setObjectName("sectionTitle");
    auto* planTitle = new QLabel("专注计划");
    planTitle->setObjectName("sectionTitle");
    auto* petTitle = new QLabel("桌宠陪伴");
    petTitle->setObjectName("sectionTitle");
    auto* growthTitle = new QLabel("成长状态");
    growthTitle->setObjectName("sectionTitle");

    auto* left = new QFrame;
    left->setObjectName("panel");
    addSoftShadow(left);
    auto* leftLayout = new QVBoxLayout(left);
    leftLayout->setContentsMargins(28, 26, 28, 22);
    leftLayout->setSpacing(12);
    leftLayout->addWidget(title);
    leftLayout->addWidget(subtitle);
    leftLayout->addSpacing(10);

    auto* workRow = new QHBoxLayout;
    workRow->setContentsMargins(0, 0, 0, 0);
    workRow->setSpacing(24);

    auto* workColumn = new QVBoxLayout;
    workColumn->setContentsMargins(0, 0, 0, 0);
    workColumn->setSpacing(12);
    workColumn->addWidget(taskTitle);
    auto* taskRow = new QHBoxLayout;
    taskRow->setContentsMargins(0, 0, 0, 0);
    taskRow->setSpacing(10);
    taskRow->addWidget(m_taskInput);
    taskRow->addWidget(taskPlanButton);
    taskRow->addStretch();
    workColumn->addLayout(taskRow);
    workColumn->addWidget(planTitle);
    workColumn->addWidget(m_planView, 1);
    workColumn->addWidget(m_statusLabel);

    workRow->addLayout(workColumn, 1);
    workRow->addWidget(m_timerPanel, 0, Qt::AlignTop);
    leftLayout->addLayout(workRow, 1);

    auto* right = new QFrame;
    right->setObjectName("sidePanel");
    right->setMinimumWidth(286);
    right->setMaximumWidth(310);
    addSoftShadow(right);
    auto* rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(24, 24, 24, 22);
    rightLayout->setSpacing(12);
    rightLayout->addWidget(petTitle);
    rightLayout->addWidget(m_petWidget, 1, Qt::AlignHCenter);
    rightLayout->addStretch();
    rightLayout->addWidget(growthTitle);
    rightLayout->addWidget(m_petProfileLabel);
    rightLayout->addWidget(m_expProgress);

    auto* root = new QWidget;
    root->setObjectName("root");
    auto* rootLayout = new QHBoxLayout(root);
    rootLayout->setContentsMargins(18, 18, 18, 18);
    rootLayout->setSpacing(16);
    rootLayout->addWidget(left, 7);
    rootLayout->addWidget(right, 3);

    setCentralWidget(root);
    updatePetProfileView();
}

void MainWindow::applyStyle() {
    // Global styling is loaded from ui/pink_theme.qss during application startup.
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

    auto listHtml = [](const QStringList& items) {
        QString html;
        if (items.isEmpty()) {
            return QString("<div class=\"empty\">暂无</div>");
        }
        html += "<ul>";
        for (const QString& item : items) {
            html += QString("<li>%1</li>").arg(item.toHtmlEscaped());
        }
        html += "</ul>";
        return html;
    };

    QString html;
    html += R"(
        <style>
            body { color: #3A2630; font-family: "Microsoft YaHei UI", "Segoe UI", Arial; font-size: 14px; }
            .task { font-size: 15px; font-weight: 700; margin-bottom: 12px; }
            .meta { margin-bottom: 14px; }
            .pill { display: inline-block; color: #D95787; background: #FFF0F6; border: 1px solid #F3C7D8; border-radius: 10px; padding: 4px 9px; margin-right: 8px; font-weight: 700; }
            .section { margin-top: 12px; padding-top: 8px; border-top: 1px solid #F6DCE6; }
            .section-title { color: #8F3156; font-weight: 800; margin-bottom: 6px; }
            ul { margin: 6px 0 0 18px; padding: 0; }
            li { margin: 5px 0; line-height: 1.45; }
            .empty { color: #8F7482; margin-top: 6px; }
        </style>
    )";
    html += QString("<div class=\"task\">%1</div>").arg(task.originalInput.toHtmlEscaped());
    html += "<div class=\"meta\">";
    html += QString("<span class=\"pill\">预计 %1 分钟</span>").arg(task.estimatedMinutes);
    html += QString("<span class=\"pill\">难度 %1</span>").arg(task.difficulty.toHtmlEscaped());
    html += "</div>";
    html += "<div class=\"section\"><div class=\"section-title\">本轮目标</div>";
    html += listHtml(task.goals);
    html += "</div>";
    html += "<div class=\"section\"><div class=\"section-title\">本轮避免</div>";
    html += listHtml(task.avoidList);
    html += "</div>";

    m_planView->setHtml(html);
    m_statusLabel->setText("计划已生成，可以开始专注。");
}

void MainWindow::updatePetProfileView() {
    const PetProfile profile = m_sessionManager.petProfile();
    m_petProfileLabel->setText(QString("Lv.%1  🍓 EXP %2/100\n♥ 亲密度 %3\n累计专注 %4 分钟\n🍅 番茄轮数 %5")
        .arg(profile.level)
        .arg(profile.exp % 100)
        .arg(profile.intimacy)
        .arg(profile.totalFocusMinutes)
        .arg(profile.completedPomodoros));
    m_expProgress->setValue(profile.exp % 100);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    hide();
    event->ignore();
}
