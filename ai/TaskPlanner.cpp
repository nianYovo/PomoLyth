#include "ai/TaskPlanner.h"

TaskPlanner::TaskPlanner(IAiClient& aiClient) : m_aiClient(aiClient) {}

FocusTask TaskPlanner::generatePlan(const QString& userInput, int minutes) {
    const QString response = m_aiClient.chat(m_promptBuilder.buildTaskPlanningPrompt(userInput));

    FocusTask task;
    task.originalInput = userInput.trimmed();
    task.estimatedMinutes = minutes;
    task.difficulty = "normal";
    task.goals = {
        "明确本轮最小可完成目标",
        "先完成最核心的一步",
        "记录一个需要复盘的卡点"
    };
    task.avoidList = {
        "不要频繁切换资料",
        "不要临时扩展任务范围",
        "不要打开娱乐软件"
    };

    if (!response.trimmed().isEmpty()) {
        task.goals[0] = response.section("避免事项", 0, 0).trimmed().left(80);
    }

    return task;
}
