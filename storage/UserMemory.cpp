#include "storage/UserMemory.h"

#include <algorithm>
#include <QDate>
#include <QMap>
#include <QSet>

UserMemory::UserMemory(SQLiteStorage& storage) : m_storage(storage) {}

UserMemorySnapshot UserMemory::buildSnapshot() const {
    UserMemorySnapshot snapshot;
    const QVector<FocusSession> sessions = m_storage.allSessions();
    const DashboardStats stats = m_storage.dashboardStats();

    QSet<QDate> focusDays;
    QMap<int, int> minutesByHour;
    QMap<QString, int> taskFrequency;

    for (const FocusSession& session : sessions) {
        snapshot.totalFocusMinutes += session.actualMinutes;
        focusDays.insert(session.createdAt.date());
        minutesByHour[session.createdAt.time().hour()] += session.actualMinutes;

        const QString task = session.task.trimmed().left(32);
        if (!task.isEmpty()) {
            taskFrequency[task] += 1;
        }
    }

    snapshot.totalFocusDays = focusDays.size();
    snapshot.commonDistractionSource = stats.commonDistractionSource;

    int bestMinutes = -1;
    for (auto it = minutesByHour.cbegin(); it != minutesByHour.cend(); ++it) {
        if (it.value() > bestMinutes) {
            bestMinutes = it.value();
            snapshot.bestFocusHour = it.key();
        }
    }

    QList<QPair<QString, int>> tasks;
    for (auto it = taskFrequency.cbegin(); it != taskFrequency.cend(); ++it) {
        tasks.append({it.key(), it.value()});
    }
    std::sort(tasks.begin(), tasks.end(), [](const auto& left, const auto& right) {
        if (left.second == right.second) {
            return left.first < right.first;
        }
        return left.second > right.second;
    });

    const int taskCount = std::min(3, static_cast<int>(tasks.size()));
    for (int i = 0; i < taskCount; ++i) {
        snapshot.frequentTasks.append(tasks[i].first);
    }

    const QString bestHourText = snapshot.bestFocusHour >= 0
        ? QString("%1:00-%2:00")
              .arg(snapshot.bestFocusHour, 2, 10, QLatin1Char('0'))
              .arg((snapshot.bestFocusHour + 1) % 24, 2, 10, QLatin1Char('0'))
        : "暂无";
    const QString sourceText = snapshot.commonDistractionSource.isEmpty() ? "暂无" : snapshot.commonDistractionSource;
    const QString tasksText = snapshot.frequentTasks.isEmpty() ? "暂无" : snapshot.frequentTasks.join(" / ");

    snapshot.summary = QString("长期记忆：累计专注 %1 分钟，覆盖 %2 天。高效时段倾向于 %3。常见任务：%4。常见分心源：%5。")
        .arg(snapshot.totalFocusMinutes)
        .arg(snapshot.totalFocusDays)
        .arg(bestHourText, tasksText, sourceText);

    return snapshot;
}
