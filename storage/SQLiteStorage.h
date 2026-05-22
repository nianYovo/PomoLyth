#pragma once

#include <QDate>
#include <QObject>
#include <QMap>
#include <QSqlDatabase>
#include <QVector>
#include "models/DistractionEvent.h"
#include "models/FocusSession.h"
#include "models/PetProfile.h"

struct DashboardStats {
    int todayMinutes = 0;
    int todayPomodoros = 0;
    int weekMinutes = 0;
    double averageDistractions = 0.0;
    QString commonDistractionSource;
};

struct DailyReport {
    int id = -1;
    QString content;
    QDateTime createdAt = QDateTime::currentDateTime();
};

class SQLiteStorage : public QObject {
    Q_OBJECT

public:
    explicit SQLiteStorage(QObject* parent = nullptr);
    ~SQLiteStorage() override;

    bool open();
    bool saveFocusSession(FocusSession& session);
    bool saveDistractionEvents(int sessionId, const QVector<DistractionEvent>& events);
    QVector<FocusSession> recentSessions(int limit = 20) const;
    QVector<FocusSession> allSessions() const;
    QVector<FocusSession> todaySessions() const;
    QVector<DistractionEvent> recentDistractions(int limit = 20) const;
    DashboardStats dashboardStats() const;
    bool saveDailyReport(const QString& content);
    QVector<DailyReport> recentDailyReports(int limit = 7) const;
    bool saveDayPlan(const QDate& date, const QString& content);
    QString dayPlan(const QDate& date) const;
    QMap<QDate, QString> monthPlans(int year, int month) const;

    PetProfile loadPetProfile() const;
    bool savePetProfile(const PetProfile& profile);
    QStringList loadAchievements() const;
    QVector<QPair<QString, QDateTime>> recentAchievements(int limit = 20) const;
    bool saveAchievement(const QString& name);

private:
    bool ensureSchema();
    QString databasePath() const;

    QString m_connectionName;
    QSqlDatabase m_db;
};
