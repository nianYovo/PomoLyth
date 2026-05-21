#include "storage/SQLiteStorage.h"

#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QUuid>
#include <QVariant>

SQLiteStorage::SQLiteStorage(QObject* parent)
    : QObject(parent),
      m_connectionName(QString("pomolyth_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces))) {}

SQLiteStorage::~SQLiteStorage() {
    if (m_db.isOpen()) {
        m_db.close();
    }
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool SQLiteStorage::open() {
    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(databasePath());
    if (!m_db.open()) {
        return false;
    }
    return ensureSchema();
}

bool SQLiteStorage::saveFocusSession(FocusSession& session) {
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO focus_sessions "
        "(task, planned_minutes, actual_minutes, distraction_count, input_activity_count, max_idle_seconds, ai_summary, next_suggestion, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(session.task);
    query.addBindValue(session.plannedMinutes);
    query.addBindValue(session.actualMinutes);
    query.addBindValue(session.distractionCount);
    query.addBindValue(session.keyboardMouseActivityCount);
    query.addBindValue(session.maxIdleSeconds);
    query.addBindValue(session.aiSummary);
    query.addBindValue(session.nextSuggestion);
    query.addBindValue(session.createdAt.toString(Qt::ISODate));
    if (!query.exec()) {
        return false;
    }
    session.id = query.lastInsertId().toInt();
    return true;
}

bool SQLiteStorage::saveDistractionEvents(int sessionId, const QVector<DistractionEvent>& events) {
    if (sessionId <= 0 || events.isEmpty()) {
        return true;
    }

    QSqlQuery query(m_db);
    m_db.transaction();
    for (const DistractionEvent& event : events) {
        query.prepare(
            "INSERT INTO distraction_events "
            "(focus_session_id, window_title, process_name, severity, created_at) "
            "VALUES (?, ?, ?, ?, ?)");
        query.addBindValue(sessionId);
        query.addBindValue(event.windowTitle);
        query.addBindValue(event.processName);
        query.addBindValue(event.severity);
        query.addBindValue(event.timestamp.toString(Qt::ISODate));
        if (!query.exec()) {
            m_db.rollback();
            return false;
        }
    }
    return m_db.commit();
}

QVector<FocusSession> SQLiteStorage::recentSessions(int limit) const {
    QVector<FocusSession> sessions;
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT id, task, planned_minutes, actual_minutes, distraction_count, input_activity_count, max_idle_seconds, ai_summary, next_suggestion, created_at "
        "FROM focus_sessions ORDER BY datetime(created_at) DESC LIMIT ?");
    query.addBindValue(limit);
    if (!query.exec()) {
        return sessions;
    }

    while (query.next()) {
        FocusSession session;
        session.id = query.value(0).toInt();
        session.task = query.value(1).toString();
        session.plannedMinutes = query.value(2).toInt();
        session.actualMinutes = query.value(3).toInt();
        session.distractionCount = query.value(4).toInt();
        session.keyboardMouseActivityCount = query.value(5).toInt();
        session.maxIdleSeconds = query.value(6).toInt();
        session.aiSummary = query.value(7).toString();
        session.nextSuggestion = query.value(8).toString();
        session.createdAt = QDateTime::fromString(query.value(9).toString(), Qt::ISODate);
        sessions.append(session);
    }
    return sessions;
}

QVector<FocusSession> SQLiteStorage::allSessions() const {
    QVector<FocusSession> sessions;
    QSqlQuery query(m_db);
    query.exec(
        "SELECT id, task, planned_minutes, actual_minutes, distraction_count, input_activity_count, max_idle_seconds, ai_summary, next_suggestion, created_at "
        "FROM focus_sessions ORDER BY datetime(created_at) DESC");

    while (query.next()) {
        FocusSession session;
        session.id = query.value(0).toInt();
        session.task = query.value(1).toString();
        session.plannedMinutes = query.value(2).toInt();
        session.actualMinutes = query.value(3).toInt();
        session.distractionCount = query.value(4).toInt();
        session.keyboardMouseActivityCount = query.value(5).toInt();
        session.maxIdleSeconds = query.value(6).toInt();
        session.aiSummary = query.value(7).toString();
        session.nextSuggestion = query.value(8).toString();
        session.createdAt = QDateTime::fromString(query.value(9).toString(), Qt::ISODate);
        sessions.append(session);
    }
    return sessions;
}

QVector<FocusSession> SQLiteStorage::todaySessions() const {
    QVector<FocusSession> sessions;
    QSqlQuery query(m_db);
    query.exec(
        "SELECT id, task, planned_minutes, actual_minutes, distraction_count, input_activity_count, max_idle_seconds, ai_summary, next_suggestion, created_at "
        "FROM focus_sessions WHERE date(created_at) = date('now', 'localtime') "
        "ORDER BY datetime(created_at) DESC");

    while (query.next()) {
        FocusSession session;
        session.id = query.value(0).toInt();
        session.task = query.value(1).toString();
        session.plannedMinutes = query.value(2).toInt();
        session.actualMinutes = query.value(3).toInt();
        session.distractionCount = query.value(4).toInt();
        session.keyboardMouseActivityCount = query.value(5).toInt();
        session.maxIdleSeconds = query.value(6).toInt();
        session.aiSummary = query.value(7).toString();
        session.nextSuggestion = query.value(8).toString();
        session.createdAt = QDateTime::fromString(query.value(9).toString(), Qt::ISODate);
        sessions.append(session);
    }
    return sessions;
}

QVector<DistractionEvent> SQLiteStorage::recentDistractions(int limit) const {
    QVector<DistractionEvent> events;
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT window_title, process_name, severity, created_at "
        "FROM distraction_events ORDER BY datetime(created_at) DESC LIMIT ?");
    query.addBindValue(limit);
    if (!query.exec()) {
        return events;
    }

    while (query.next()) {
        DistractionEvent event;
        event.windowTitle = query.value(0).toString();
        event.processName = query.value(1).toString();
        event.severity = query.value(2).toInt();
        event.timestamp = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);
        events.append(event);
    }
    return events;
}

DashboardStats SQLiteStorage::dashboardStats() const {
    DashboardStats stats;

    QSqlQuery today(m_db);
    today.exec(
        "SELECT COALESCE(SUM(actual_minutes), 0), COUNT(*) "
        "FROM focus_sessions WHERE date(created_at) = date('now', 'localtime')");
    if (today.next()) {
        stats.todayMinutes = today.value(0).toInt();
        stats.todayPomodoros = today.value(1).toInt();
    }

    QSqlQuery week(m_db);
    week.exec(
        "SELECT COALESCE(SUM(actual_minutes), 0), COALESCE(AVG(distraction_count), 0) "
        "FROM focus_sessions WHERE datetime(created_at) >= datetime('now', '-7 days', 'localtime')");
    if (week.next()) {
        stats.weekMinutes = week.value(0).toInt();
        stats.averageDistractions = week.value(1).toDouble();
    }

    QSqlQuery source(m_db);
    source.exec(
        "SELECT COALESCE(NULLIF(process_name, ''), window_title) AS source, COUNT(*) AS count "
        "FROM distraction_events GROUP BY source ORDER BY count DESC LIMIT 1");
    if (source.next()) {
        stats.commonDistractionSource = source.value(0).toString();
    }

    return stats;
}

bool SQLiteStorage::saveDailyReport(const QString& content) {
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO daily_reports (content, created_at) VALUES (?, ?)");
    query.addBindValue(content);
    query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    return query.exec();
}

QVector<DailyReport> SQLiteStorage::recentDailyReports(int limit) const {
    QVector<DailyReport> reports;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, content, created_at FROM daily_reports ORDER BY datetime(created_at) DESC LIMIT ?");
    query.addBindValue(limit);
    if (!query.exec()) {
        return reports;
    }

    while (query.next()) {
        DailyReport report;
        report.id = query.value(0).toInt();
        report.content = query.value(1).toString();
        report.createdAt = QDateTime::fromString(query.value(2).toString(), Qt::ISODate);
        reports.append(report);
    }
    return reports;
}

PetProfile SQLiteStorage::loadPetProfile() const {
    PetProfile profile;
    QSqlQuery query(m_db);
    query.exec("SELECT level, exp, intimacy, energy, total_focus_minutes, completed_pomodoros FROM pet_profile WHERE id = 1");
    if (query.next()) {
        profile.level = query.value(0).toInt();
        profile.exp = query.value(1).toInt();
        profile.intimacy = query.value(2).toInt();
        profile.energy = query.value(3).toInt();
        profile.totalFocusMinutes = query.value(4).toInt();
        profile.completedPomodoros = query.value(5).toInt();
    }
    profile.achievements = loadAchievements();
    return profile;
}

bool SQLiteStorage::savePetProfile(const PetProfile& profile) {
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO pet_profile (id, level, exp, intimacy, energy, total_focus_minutes, completed_pomodoros) "
        "VALUES (1, ?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(id) DO UPDATE SET "
        "level = excluded.level, exp = excluded.exp, intimacy = excluded.intimacy, energy = excluded.energy, "
        "total_focus_minutes = excluded.total_focus_minutes, completed_pomodoros = excluded.completed_pomodoros");
    query.addBindValue(profile.level);
    query.addBindValue(profile.exp);
    query.addBindValue(profile.intimacy);
    query.addBindValue(profile.energy);
    query.addBindValue(profile.totalFocusMinutes);
    query.addBindValue(profile.completedPomodoros);
    return query.exec();
}

QStringList SQLiteStorage::loadAchievements() const {
    QStringList achievements;
    QSqlQuery query(m_db);
    query.exec("SELECT name FROM achievements ORDER BY datetime(unlocked_at) ASC");
    while (query.next()) {
        achievements.append(query.value(0).toString());
    }
    return achievements;
}

QVector<QPair<QString, QDateTime>> SQLiteStorage::recentAchievements(int limit) const {
    QVector<QPair<QString, QDateTime>> achievements;
    QSqlQuery query(m_db);
    query.prepare("SELECT name, unlocked_at FROM achievements ORDER BY datetime(unlocked_at) DESC LIMIT ?");
    query.addBindValue(limit);
    if (!query.exec()) {
        return achievements;
    }

    while (query.next()) {
        achievements.append({
            query.value(0).toString(),
            QDateTime::fromString(query.value(1).toString(), Qt::ISODate)
        });
    }
    return achievements;
}

bool SQLiteStorage::saveAchievement(const QString& name) {
    QSqlQuery query(m_db);
    query.prepare("INSERT OR IGNORE INTO achievements (name, unlocked_at) VALUES (?, ?)");
    query.addBindValue(name);
    query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    return query.exec();
}

bool SQLiteStorage::ensureSchema() {
    QSqlQuery query(m_db);
    const bool sessionsOk = query.exec(
        "CREATE TABLE IF NOT EXISTS focus_sessions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "task TEXT NOT NULL,"
        "planned_minutes INTEGER NOT NULL,"
        "actual_minutes INTEGER NOT NULL,"
        "distraction_count INTEGER NOT NULL,"
        "input_activity_count INTEGER NOT NULL DEFAULT 0,"
        "max_idle_seconds INTEGER NOT NULL DEFAULT 0,"
        "ai_summary TEXT,"
        "next_suggestion TEXT,"
        "created_at TEXT NOT NULL)");

    query.exec("ALTER TABLE focus_sessions ADD COLUMN input_activity_count INTEGER NOT NULL DEFAULT 0");
    query.exec("ALTER TABLE focus_sessions ADD COLUMN max_idle_seconds INTEGER NOT NULL DEFAULT 0");

    const bool petOk = query.exec(
        "CREATE TABLE IF NOT EXISTS pet_profile ("
        "id INTEGER PRIMARY KEY,"
        "level INTEGER NOT NULL,"
        "exp INTEGER NOT NULL,"
        "intimacy INTEGER NOT NULL,"
        "energy INTEGER NOT NULL,"
        "total_focus_minutes INTEGER NOT NULL,"
        "completed_pomodoros INTEGER NOT NULL)");

    const bool distractionsOk = query.exec(
        "CREATE TABLE IF NOT EXISTS distraction_events ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "focus_session_id INTEGER NOT NULL,"
        "window_title TEXT,"
        "process_name TEXT,"
        "severity INTEGER NOT NULL,"
        "created_at TEXT NOT NULL,"
        "FOREIGN KEY(focus_session_id) REFERENCES focus_sessions(id))");

    const bool achievementsOk = query.exec(
        "CREATE TABLE IF NOT EXISTS achievements ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL UNIQUE,"
        "unlocked_at TEXT NOT NULL)");

    const bool reportsOk = query.exec(
        "CREATE TABLE IF NOT EXISTS daily_reports ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "content TEXT NOT NULL,"
        "created_at TEXT NOT NULL)");

    return sessionsOk && petOk && distractionsOk && achievementsOk && reportsOk;
}

QString SQLiteStorage::databasePath() const {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.isEmpty()) {
        dir = QDir::currentPath();
    }
    QDir().mkpath(dir);
    return QDir(dir).filePath("pomolyth.sqlite");
}
