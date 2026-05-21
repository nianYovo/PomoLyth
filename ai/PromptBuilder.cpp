#include "ai/PromptBuilder.h"

QString PromptBuilder::buildTaskPlanningPrompt(const QString& userInput) const {
    return QString(
        "你是一个专注规划助手。请把用户任务拆成适合一轮番茄钟完成的计划，包含目标、避免事项、难度和预计分钟数。\n"
        "用户任务：%1").arg(userInput);
}

QString PromptBuilder::buildReviewPrompt(const FocusSession& session) const {
    return QString(
        "请为这轮番茄钟生成简短复盘。\n任务：%1\n计划时长：%2\n实际时长：%3\n分心次数：%4\n完成：%5\n问题：%6")
        .arg(session.task)
        .arg(session.plannedMinutes)
        .arg(session.actualMinutes)
        .arg(session.distractionCount)
        .arg(session.completedGoals.join("；"))
        .arg(session.problems.join("；"));
}

QString PromptBuilder::buildDailyReportPrompt(const QVector<FocusSession>& sessions) const {
    int minutes = 0;
    for (const auto& session : sessions) {
        minutes += session.actualMinutes;
    }
    return QString("请根据今天 %1 轮、共 %2 分钟专注记录生成日报。").arg(sessions.size()).arg(minutes);
}

QString PromptBuilder::buildDailyReportPrompt(const QVector<FocusSession>& sessions, const QString& memorySummary) const {
    return buildDailyReportPrompt(sessions) + QString("\n长期记忆：%1").arg(memorySummary);
}
