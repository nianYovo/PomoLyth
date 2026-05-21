#include "ai/PromptBuilder.h"

QString PromptBuilder::buildTaskPlanningPrompt(const QString& userInput) const {
    return QString(
        "You are a focus planning assistant. Split the user task into one pomodoro-sized plan with goals, avoid-list, difficulty, and estimated minutes.\n"
        "User task: %1").arg(userInput);
}

QString PromptBuilder::buildReviewPrompt(const FocusSession& session) const {
    return QString(
        "Generate a short review for this pomodoro.\nTask: %1\nPlanned minutes: %2\nActual minutes: %3\nDistractions: %4\nCompleted: %5\nProblems: %6")
        .arg(session.task)
        .arg(session.plannedMinutes)
        .arg(session.actualMinutes)
        .arg(session.distractionCount)
        .arg(session.completedGoals.join("; "))
        .arg(session.problems.join("; "));
}

QString PromptBuilder::buildDailyReportPrompt(const QVector<FocusSession>& sessions) const {
    int minutes = 0;
    for (const auto& session : sessions) {
        minutes += session.actualMinutes;
    }
    return QString("Generate a daily focus report from %1 sessions and %2 focused minutes.").arg(sessions.size()).arg(minutes);
}

QString PromptBuilder::buildDailyReportPrompt(const QVector<FocusSession>& sessions, const QString& memorySummary) const {
    return buildDailyReportPrompt(sessions) + QString("\nLong-term memory: %1").arg(memorySummary);
}
