#include "ui/DashboardWindow.h"

#include <QFile>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QtConcurrent>

static QLabel* metricLabel(const QString& title) {
    auto* label = new QLabel(QString("%1\n0").arg(title));
    label->setAlignment(Qt::AlignCenter);
    label->setObjectName("metricLabel");
    return label;
}

static QString csvEscape(QString value) {
    value.replace('"', "\"\"");
    if (value.contains(',') || value.contains('\n') || value.contains('"')) {
        return QString("\"%1\"").arg(value);
    }
    return value;
}

DashboardWindow::DashboardWindow(SQLiteStorage& storage, ReviewGenerator& reviewGenerator, QWidget* parent)
    : QWidget(parent), m_storage(storage), m_reviewGenerator(reviewGenerator), m_userMemory(storage) {
    setWindowTitle("PomoLyth 数据面板");
    resize(820, 620);

    m_todayMinutes = metricLabel("今日分钟");
    m_todayPomodoros = metricLabel("今日番茄");
    m_weekMinutes = metricLabel("近 7 天分钟");
    m_averageDistractions = metricLabel("平均分心");
    m_commonDistraction = metricLabel("常见来源");

    auto* metrics = new QGridLayout;
    metrics->addWidget(m_todayMinutes, 0, 0);
    metrics->addWidget(m_todayPomodoros, 0, 1);
    metrics->addWidget(m_weekMinutes, 0, 2);
    metrics->addWidget(m_averageDistractions, 0, 3);
    metrics->addWidget(m_commonDistraction, 0, 4);

    m_dailyReport = new QLabel("今日报告尚未生成。");
    m_dailyReport->setObjectName("dailyReportLabel");
    m_dailyReport->setWordWrap(true);
    m_userMemoryLabel = new QLabel("长期记忆正在等待更多记录。");
    m_userMemoryLabel->setObjectName("dailyReportLabel");
    m_userMemoryLabel->setWordWrap(true);
    m_dailyReportButton = new QPushButton("生成今日报告");
    m_exportButton = new QPushButton("导出 CSV");

    connect(m_dailyReportButton, &QPushButton::clicked, this, [this]() {
        const QVector<FocusSession> sessions = m_storage.todaySessions();
        const QString memorySummary = m_userMemory.buildSnapshot().summary;
        m_dailyReportButton->setEnabled(false);
        m_dailyReport->setText("正在生成今日报告...");

        auto* watcher = new QFutureWatcher<QString>(this);
        connect(watcher, &QFutureWatcher<QString>::finished, this, [this, watcher]() {
            const QString report = watcher->result();
            watcher->deleteLater();
            m_storage.saveDailyReport(report);
            m_dailyReport->setText(report);
            m_dailyReportButton->setEnabled(true);
            refresh();
        });
        watcher->setFuture(QtConcurrent::run([this, sessions, memorySummary]() {
            return m_reviewGenerator.generateDailyReport(sessions, memorySummary);
        }));
    });
    connect(m_exportButton, &QPushButton::clicked, this, &DashboardWindow::exportCsv);

    m_table = new QTableWidget(0, 7);
    m_table->setHorizontalHeaderLabels({"时间", "任务", "计划", "实际", "分心", "键鼠活动", "最长空闲"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_distractionTable = new QTableWidget(0, 4);
    m_distractionTable->setHorizontalHeaderLabels({"时间", "进程", "窗口", "等级"});
    m_distractionTable->horizontalHeader()->setStretchLastSection(true);
    m_distractionTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_distractionTable->verticalHeader()->setVisible(false);
    m_distractionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_achievementTable = new QTableWidget(0, 2);
    m_achievementTable->setHorizontalHeaderLabels({"时间", "成就"});
    m_achievementTable->horizontalHeader()->setStretchLastSection(true);
    m_achievementTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_achievementTable->verticalHeader()->setVisible(false);
    m_achievementTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(metrics);
    layout->addWidget(m_dailyReportButton);
    layout->addWidget(m_exportButton);
    layout->addWidget(m_userMemoryLabel);
    layout->addWidget(m_dailyReport);
    layout->addWidget(new QLabel("最近专注记录"));
    layout->addWidget(m_table);
    layout->addWidget(new QLabel("最近分心记录"));
    layout->addWidget(m_distractionTable);
    layout->addWidget(new QLabel("最近解锁成就"));
    layout->addWidget(m_achievementTable);

    refresh();
}

void DashboardWindow::refresh() {
    const DashboardStats stats = m_storage.dashboardStats();
    const UserMemorySnapshot memory = m_userMemory.buildSnapshot();
    m_userMemoryLabel->setText(memory.summary);
    m_todayMinutes->setText(QString("今日分钟\n%1").arg(stats.todayMinutes));
    m_todayPomodoros->setText(QString("今日番茄\n%1").arg(stats.todayPomodoros));
    m_weekMinutes->setText(QString("近 7 天分钟\n%1").arg(stats.weekMinutes));
    m_averageDistractions->setText(QString("平均分心\n%1").arg(stats.averageDistractions, 0, 'f', 1));
    m_commonDistraction->setText(QString("常见来源\n%1").arg(stats.commonDistractionSource.isEmpty() ? "暂无" : stats.commonDistractionSource));

    const QVector<DailyReport> reports = m_storage.recentDailyReports(1);
    if (!reports.isEmpty()) {
        m_dailyReport->setText(QString("最近日报 %1\n%2")
            .arg(reports.first().createdAt.toString("MM-dd HH:mm"), reports.first().content));
    }

    const QVector<FocusSession> sessions = m_storage.recentSessions();
    m_table->setRowCount(sessions.size());
    for (int row = 0; row < sessions.size(); ++row) {
        const FocusSession& session = sessions[row];
        m_table->setItem(row, 0, new QTableWidgetItem(session.createdAt.toString("MM-dd HH:mm")));
        m_table->setItem(row, 1, new QTableWidgetItem(session.task));
        m_table->setItem(row, 2, new QTableWidgetItem(QString::number(session.plannedMinutes)));
        m_table->setItem(row, 3, new QTableWidgetItem(QString::number(session.actualMinutes)));
        m_table->setItem(row, 4, new QTableWidgetItem(QString::number(session.distractionCount)));
        m_table->setItem(row, 5, new QTableWidgetItem(QString::number(session.keyboardMouseActivityCount)));
        m_table->setItem(row, 6, new QTableWidgetItem(QString("%1 秒").arg(session.maxIdleSeconds)));
    }

    const QVector<DistractionEvent> distractions = m_storage.recentDistractions();
    m_distractionTable->setRowCount(distractions.size());
    for (int row = 0; row < distractions.size(); ++row) {
        const DistractionEvent& event = distractions[row];
        m_distractionTable->setItem(row, 0, new QTableWidgetItem(event.timestamp.toString("MM-dd HH:mm")));
        m_distractionTable->setItem(row, 1, new QTableWidgetItem(event.processName));
        m_distractionTable->setItem(row, 2, new QTableWidgetItem(event.windowTitle));
        m_distractionTable->setItem(row, 3, new QTableWidgetItem(QString::number(event.severity)));
    }

    const QVector<QPair<QString, QDateTime>> achievements = m_storage.recentAchievements();
    m_achievementTable->setRowCount(achievements.size());
    for (int row = 0; row < achievements.size(); ++row) {
        m_achievementTable->setItem(row, 0, new QTableWidgetItem(achievements[row].second.toString("MM-dd HH:mm")));
        m_achievementTable->setItem(row, 1, new QTableWidgetItem(achievements[row].first));
    }
}

void DashboardWindow::exportCsv() {
    const QString path = QFileDialog::getSaveFileName(
        this,
        "导出专注记录",
        "pomolyth_sessions.csv",
        "CSV Files (*.csv)");
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::warning(this, "导出失败", "无法写入 CSV 文件。");
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << "created_at,task,planned_minutes,actual_minutes,distraction_count,input_activity_count,max_idle_seconds,ai_summary,next_suggestion\n";
    for (const FocusSession& session : m_storage.allSessions()) {
        out << csvEscape(session.createdAt.toString(Qt::ISODate)) << ','
            << csvEscape(session.task) << ','
            << session.plannedMinutes << ','
            << session.actualMinutes << ','
            << session.distractionCount << ','
            << session.keyboardMouseActivityCount << ','
            << session.maxIdleSeconds << ','
            << csvEscape(session.aiSummary) << ','
            << csvEscape(session.nextSuggestion) << '\n';
    }

    QMessageBox::information(this, "导出完成", "专注记录已导出。");
}
