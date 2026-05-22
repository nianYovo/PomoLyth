#include "ui/DashboardWindow.h"

#include <QAbstractItemView>
#include <QColor>
#include <QFile>
#include <QFileDialog>
#include <QFutureWatcher>
#include <QFrame>
#include <QGridLayout>
#include <QGraphicsDropShadowEffect>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTextStream>
#include <QVBoxLayout>
#include <QtConcurrent>

static QLabel* metricLabel(const QString& title) {
    auto* label = new QLabel(QString("%1\n0").arg(title));
    label->setAlignment(Qt::AlignCenter);
    label->setObjectName("metricLabel");
    label->setMinimumSize(138, 82);
    return label;
}

static QLabel* sectionTitle(const QString& title) {
    auto* label = new QLabel(title);
    label->setObjectName("sectionTitle");
    return label;
}

static QFrame* reportCard(const QString& title, QLabel* content) {
    auto* card = new QFrame;
    card->setObjectName("reportCard");

    auto* titleLabel = sectionTitle(title);
    content->setObjectName("reportText");
    content->setMinimumHeight(74);

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 14, 16, 16);
    layout->setSpacing(8);
    layout->addWidget(titleLabel);
    layout->addWidget(content, 1);
    return card;
}

static void addSoftShadow(QWidget* widget, int blurRadius = 18) {
    auto* shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(blurRadius);
    shadow->setOffset(0, 6);
    shadow->setColor(QColor(255, 141, 179, 36));
    widget->setGraphicsEffect(shadow);
}

static void prepareTable(QTableWidget* table) {
    table->setObjectName("dataTable");
    table->setAlternatingRowColors(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setMinimumHeight(112);
    table->setMaximumHeight(152);
    table->verticalHeader()->setDefaultSectionSize(38);
    table->horizontalHeader()->setFixedHeight(40);
}

static void setEmptyTableMessage(QTableWidget* table, int columns) {
    table->clearSpans();
    table->setRowCount(1);
    table->setSpan(0, 0, 1, columns);
    auto* item = new QTableWidgetItem("还没有记录。完成一轮番茄后，这里会长出你的专注足迹 🌱");
    item->setTextAlignment(Qt::AlignCenter);
    item->setFlags(Qt::ItemIsEnabled);
    table->setItem(0, 0, item);
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
    setObjectName("dashboardWindow");
    setWindowTitle("PomoLyth 数据面板");
    resize(860, 660);

    m_todayMinutes = metricLabel("今日分钟");
    m_todayPomodoros = metricLabel("今日番茄");
    m_weekMinutes = metricLabel("近 7 天分钟");
    m_averageDistractions = metricLabel("平均分心");
    m_commonDistraction = metricLabel("常见来源");
    for (auto* label : {m_todayMinutes, m_todayPomodoros, m_weekMinutes, m_averageDistractions, m_commonDistraction}) {
        addSoftShadow(label);
    }

    auto* metrics = new QGridLayout;
    metrics->setHorizontalSpacing(12);
    metrics->setVerticalSpacing(12);
    metrics->addWidget(m_todayMinutes, 0, 0);
    metrics->addWidget(m_todayPomodoros, 0, 1);
    metrics->addWidget(m_weekMinutes, 0, 2);
    metrics->addWidget(m_averageDistractions, 1, 0);
    metrics->addWidget(m_commonDistraction, 1, 1, 1, 2);
    for (int column = 0; column < 3; ++column) {
        metrics->setColumnStretch(column, 1);
    }

    m_dailyReport = new QLabel("今日报告尚未生成。");
    m_dailyReport->setObjectName("dailyReportLabel");
    m_dailyReport->setWordWrap(true);
    m_userMemoryLabel = new QLabel("长期记忆正在等待更多记录。");
    m_userMemoryLabel->setObjectName("dailyReportLabel");
    m_userMemoryLabel->setWordWrap(true);

    m_dailyReportButton = new QPushButton("生成今日报告");
    m_dailyReportButton->setProperty("buttonRole", "primary");
    m_exportButton = new QPushButton("导出 CSV");
    m_exportButton->setProperty("buttonRole", "outline");

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

    m_table = new QTableWidget(0, 5);
    prepareTable(m_table);
    m_table->setHorizontalHeaderLabels({"时间", "任务", "计划", "实际", "分心"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    m_distractionTable = new QTableWidget(0, 4);
    prepareTable(m_distractionTable);
    m_distractionTable->setHorizontalHeaderLabels({"时间", "进程", "窗口", "等级"});
    m_distractionTable->horizontalHeader()->setStretchLastSection(true);
    m_distractionTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    m_achievementTable = new QTableWidget(0, 2);
    prepareTable(m_achievementTable);
    m_achievementTable->setHorizontalHeaderLabels({"时间", "成就"});
    m_achievementTable->horizontalHeader()->setStretchLastSection(true);
    m_achievementTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    auto* buttonRow = new QHBoxLayout;
    buttonRow->setSpacing(10);
    buttonRow->addWidget(m_dailyReportButton);
    buttonRow->addWidget(m_exportButton);
    buttonRow->addStretch();

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(14);
    layout->addLayout(metrics);
    layout->addLayout(buttonRow);
    auto* reportRow = new QHBoxLayout;
    reportRow->setSpacing(14);
    reportRow->addWidget(reportCard("今日复盘小纸条", m_dailyReport));
    reportRow->addWidget(reportCard("长期记忆", m_userMemoryLabel));
    layout->addLayout(reportRow);
    layout->addWidget(sectionTitle("最近专注记录"));
    layout->addWidget(m_table);
    layout->addWidget(sectionTitle("最近分心记录"));
    layout->addWidget(m_distractionTable);
    layout->addWidget(sectionTitle("最近解锁成就"));
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
    m_table->clearSpans();
    m_table->setRowCount(sessions.size());
    for (int row = 0; row < sessions.size(); ++row) {
        const FocusSession& session = sessions[row];
        m_table->setItem(row, 0, new QTableWidgetItem(session.createdAt.toString("MM-dd HH:mm")));
        m_table->setItem(row, 1, new QTableWidgetItem(session.task));
        m_table->setItem(row, 2, new QTableWidgetItem(QString::number(session.plannedMinutes)));
        m_table->setItem(row, 3, new QTableWidgetItem(QString::number(session.actualMinutes)));
        m_table->setItem(row, 4, new QTableWidgetItem(QString::number(session.distractionCount)));
        m_table->setRowHeight(row, 38);
    }
    if (sessions.isEmpty()) {
        setEmptyTableMessage(m_table, 5);
        m_table->setRowHeight(0, 72);
    }

    const QVector<DistractionEvent> distractions = m_storage.recentDistractions();
    m_distractionTable->clearSpans();
    m_distractionTable->setRowCount(distractions.size());
    for (int row = 0; row < distractions.size(); ++row) {
        const DistractionEvent& event = distractions[row];
        m_distractionTable->setItem(row, 0, new QTableWidgetItem(event.timestamp.toString("MM-dd HH:mm")));
        m_distractionTable->setItem(row, 1, new QTableWidgetItem(event.processName));
        m_distractionTable->setItem(row, 2, new QTableWidgetItem(event.windowTitle));
        m_distractionTable->setItem(row, 3, new QTableWidgetItem(QString::number(event.severity)));
        m_distractionTable->setRowHeight(row, 38);
    }
    if (distractions.isEmpty()) {
        setEmptyTableMessage(m_distractionTable, 4);
        m_distractionTable->setRowHeight(0, 72);
    }

    const QVector<QPair<QString, QDateTime>> achievements = m_storage.recentAchievements();
    m_achievementTable->clearSpans();
    m_achievementTable->setRowCount(achievements.size());
    for (int row = 0; row < achievements.size(); ++row) {
        m_achievementTable->setItem(row, 0, new QTableWidgetItem(achievements[row].second.toString("MM-dd HH:mm")));
        m_achievementTable->setItem(row, 1, new QTableWidgetItem(achievements[row].first));
        m_achievementTable->setRowHeight(row, 38);
    }
    if (achievements.isEmpty()) {
        setEmptyTableMessage(m_achievementTable, 2);
        m_achievementTable->setRowHeight(0, 72);
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
    out << "created_at,task,planned_minutes,actual_minutes,distraction_count,ai_summary,next_suggestion\n";
    for (const FocusSession& session : m_storage.allSessions()) {
        out << csvEscape(session.createdAt.toString(Qt::ISODate)) << ','
            << csvEscape(session.task) << ','
            << session.plannedMinutes << ','
            << session.actualMinutes << ','
            << session.distractionCount << ','
            << csvEscape(session.aiSummary) << ','
            << csvEscape(session.nextSuggestion) << '\n';
    }

    QMessageBox::information(this, "导出完成", "专注记录已导出。");
}
