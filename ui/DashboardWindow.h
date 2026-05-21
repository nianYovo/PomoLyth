#pragma once

#include <QWidget>
#include "ai/ReviewGenerator.h"
#include "storage/SQLiteStorage.h"
#include "storage/UserMemory.h"

class QLabel;
class QPushButton;
class QTableWidget;

class DashboardWindow : public QWidget {
    Q_OBJECT

public:
    explicit DashboardWindow(SQLiteStorage& storage, ReviewGenerator& reviewGenerator, QWidget* parent = nullptr);

public slots:
    void refresh();

private:
    void exportCsv();

    SQLiteStorage& m_storage;
    ReviewGenerator& m_reviewGenerator;
    UserMemory m_userMemory;
    QLabel* m_todayMinutes = nullptr;
    QLabel* m_todayPomodoros = nullptr;
    QLabel* m_weekMinutes = nullptr;
    QLabel* m_averageDistractions = nullptr;
    QLabel* m_commonDistraction = nullptr;
    QLabel* m_dailyReport = nullptr;
    QLabel* m_userMemoryLabel = nullptr;
    QPushButton* m_dailyReportButton = nullptr;
    QPushButton* m_exportButton = nullptr;
    QTableWidget* m_table = nullptr;
    QTableWidget* m_distractionTable = nullptr;
    QTableWidget* m_achievementTable = nullptr;
};
