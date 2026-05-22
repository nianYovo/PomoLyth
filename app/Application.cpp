#include "app/Application.h"

#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QMenu>
#include <QMessageBox>
#include <QStyle>
#include <QTextStream>
#include "monitor/WindowsWindowMonitor.h"

Application::Application(QObject* parent)
    : QObject(parent),
      m_eventBus(this),
      m_timer(this),
      m_storage(this),
      m_jsonStorage("config"),
      m_achievementSystem(this) {}

bool Application::initialize() {
    loadTheme();

    if (!m_storage.open()) {
        QMessageBox::critical(nullptr, "PomoLyth", "Failed to open local SQLite database.");
        return false;
    }

    m_config = m_jsonStorage.loadAppConfig();
    setupAiClient();

    m_focusMonitor = std::make_unique<FocusMonitor>(
        m_eventBus,
        std::make_unique<WindowsWindowMonitor>());
    m_focusMonitor->setIntervalSeconds(m_config.focusMonitorIntervalSeconds);
    m_focusMonitor->setBlacklist(m_jsonStorage.loadBlacklist());
    m_focusMonitor->setWhitelist(m_jsonStorage.loadWhitelist());

    m_sessionManager = std::make_unique<FocusSessionManager>(
        m_eventBus,
        m_timer,
        *m_taskPlanner,
        *m_reviewGenerator,
        *m_focusMonitor,
        m_storage,
        m_achievementSystem);
    m_sessionManager->setBreakMinutes(m_config.defaultBreakMinutes);

    m_petStateMachine = std::make_unique<PetStateMachine>(m_eventBus);

    m_mainWindow = std::make_unique<MainWindow>(
        *m_sessionManager,
        m_timer,
        *m_petStateMachine,
        m_storage,
        m_jsonStorage,
        *m_focusMonitor,
        *m_reviewGenerator,
        m_config);

    setupTray();
    setupNotifications();
    return true;
}

void Application::loadTheme() {
    const QStringList themePaths = {
        QDir(QCoreApplication::applicationDirPath()).filePath("ui/pink_theme.qss"),
        QDir::current().filePath("ui/pink_theme.qss"),
    };

    for (const QString& path : themePaths) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);
        qApp->setStyleSheet(stream.readAll());
        return;
    }
}

void Application::show() {
    if (m_mainWindow) {
        m_mainWindow->show();
    }
}

void Application::setupAiClient() {
    if (m_config.aiProvider.compare("openai", Qt::CaseInsensitive) == 0) {
        m_aiClient = std::make_unique<OpenAiClient>(m_config);
    } else {
        m_aiClient = std::make_unique<MockAiClient>();
    }

    m_taskPlanner = std::make_unique<TaskPlanner>(*m_aiClient);
    m_reviewGenerator = std::make_unique<ReviewGenerator>(*m_aiClient);
}

void Application::setupTray() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }

    m_trayIcon = std::make_unique<QSystemTrayIcon>();
    m_trayIcon->setIcon(qApp->style()->standardIcon(QStyle::SP_ComputerIcon));
    m_trayIcon->setToolTip("PomoLyth");

    auto* menu = new QMenu;
    auto* showAction = menu->addAction("显示主窗口");
    auto* hideAction = menu->addAction("隐藏主窗口");
    menu->addSeparator();
    auto* quitAction = menu->addAction("退出");

    connect(showAction, &QAction::triggered, this, [this]() {
        if (m_mainWindow) {
            m_mainWindow->show();
            m_mainWindow->raise();
            m_mainWindow->activateWindow();
        }
    });
    connect(hideAction, &QAction::triggered, this, [this]() {
        if (m_mainWindow) {
            m_mainWindow->hide();
        }
    });
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    connect(m_trayIcon.get(), &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger && m_mainWindow) {
            m_mainWindow->setVisible(!m_mainWindow->isVisible());
            if (m_mainWindow->isVisible()) {
                m_mainWindow->raise();
                m_mainWindow->activateWindow();
            }
        }
    });

    m_trayIcon->setContextMenu(menu);
    m_trayIcon->show();
}

void Application::setupNotifications() {
    connect(&m_eventBus, &EventBus::eventPublished, this, [this](const AppEvent& event) {
        if (!m_trayIcon) {
            return;
        }

        switch (event.type) {
        case AppEventType::TimerStarted:
            m_trayIcon->showMessage("PomoLyth", "专注开始，桌宠会陪你守住这一轮。", QSystemTrayIcon::Information, 2500);
            break;
        case AppEventType::DistractionDetected:
            m_trayIcon->showMessage("PomoLyth", QString("检测到可能分心：%1").arg(event.payload), QSystemTrayIcon::Warning, 3000);
            break;
        case AppEventType::TimerCompleted:
            m_trayIcon->showMessage("PomoLyth", "计时结束，记得做一次简短复盘。", QSystemTrayIcon::Information, 3000);
            break;
        case AppEventType::BreakStarted:
            m_trayIcon->showMessage("PomoLyth", QString("进入 %1 分钟短休息。").arg(event.payload), QSystemTrayIcon::Information, 2500);
            break;
        case AppEventType::BreakFinished:
            m_trayIcon->showMessage("PomoLyth", "短休息结束，可以准备下一轮。", QSystemTrayIcon::Information, 2500);
            break;
        case AppEventType::SessionSaved:
            m_trayIcon->showMessage("PomoLyth", "本轮记录已保存。", QSystemTrayIcon::Information, 2500);
            break;
        case AppEventType::AchievementUnlocked:
            m_trayIcon->showMessage("PomoLyth", QString("解锁成就：%1").arg(event.payload), QSystemTrayIcon::Information, 3000);
            break;
        default:
            break;
        }
    });
}
