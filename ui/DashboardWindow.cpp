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
    setWindowTitle("PomoLyth Dashboard");
    resize(820, 620);

    m_todayMinutes = metricLabel("Today minutes");
    m_todayPomodoros = metricLabel("Today rounds");
    m_weekMinutes = metricLabel("7-day minutes");
    m_averageDistractions = metricLabel("Avg distractions");
    m_commonDistraction = metricLabel("Common source");

    auto* metrics = new QGridLayout;
    metrics->addWidget(m_todayMinutes, 0, 0);
    metrics->addWidget(m_todayPomodoros, 0, 1);
    metrics->addWidget(m_weekMinutes, 0, 2);
    metrics->addWidget(m_averageDistractions, 0, 3);
    metrics->addWidget(m_commonDistraction, 0, 4);

    m_dailyReport = new QLabel("No daily report generated yet.");
    m_dailyReport->setObjectName("dailyReportLabel");
    m_dailyReport->setWordWrap(true);
    m_userMemoryLabel = new QLabel("Long-term memory is waiting for more records.");
    m_userMemoryLabel->setObjectName("dailyReportLabel");
    m_userMemoryLabel->setWordWrap(true);
    m_dailyReportButton = new QPushButton("Generate daily report");
    m_exportButton = new QPushButton("Export CSV");

    connect(m_dailyReportButton, &QPushButton::clicked, this, [this]() {
        const QVector<FocusSession> sessions = m_storage.todaySessions();
        const QString memorySummary = m_userMemory.buildSnapshot().summary;
        m_dailyReportButton->setEnabled(false);
        m_dailyReport->setText("Generating daily report...");

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
    m_table->setHorizontalHeaderLabels({"Time", "Task", "Planned", "Actual", "Distractions"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_distractionTable = new QTableWidget(0, 4);
    m_distractionTable->setHorizontalHeaderLabels({"Time", "Process", "Window", "Severity"});
    m_distractionTable->horizontalHeader()->setStretchLastSection(true);
    m_distractionTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_distractionTable->verticalHeader()->setVisible(false);
    m_distractionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_achievementTable = new QTableWidget(0, 2);
    m_achievementTable->setHorizontalHeaderLabels({"Time", "Achievement"});
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
    layout->addWidget(new QLabel("Recent sessions"));
    layout->addWidget(m_table);
    layout->addWidget(new QLabel("Recent distractions"));
    layout->addWidget(m_distractionTable);
    layout->addWidget(new QLabel("Recent achievements"));
    layout->addWidget(m_achievementTable);

    refresh();
}

void DashboardWindow::refresh() {
    const DashboardStats stats = m_storage.dashboardStats();
    const UserMemorySnapshot memory = m_userMemory.buildSnapshot();
    m_userMemoryLabel->setText(memory.summary);
    m_todayMinutes->setText(QString("Today minutes\n%1").arg(stats.todayMinutes));
    m_todayPomodoros->setText(QString("Today rounds\n%1").arg(stats.todayPomodoros));
    m_weekMinutes->setText(QString("7-day minutes\n%1").arg(stats.weekMinutes));
    m_averageDistractions->setText(QString("Avg distractions\n%1").arg(stats.averageDistractions, 0, 'f', 1));
    m_commonDistraction->setText(QString("Common source\n%1").arg(stats.commonDistractionSource.isEmpty() ? "None" : stats.commonDistractionSource));

    const QVector<DailyReport> reports = m_storage.recentDailyReports(1);
    if (!reports.isEmpty()) {
        m_dailyReport->setText(QString("Latest report %1\n%2")
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
        "Export focus sessions",
        "pomolyth_sessions.csv",
        "CSV Files (*.csv)");
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::warning(this, "Export failed", "Unable to write the CSV file.");
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

    QMessageBox::information(this, "Export complete", "Focus sessions were exported.");
}
