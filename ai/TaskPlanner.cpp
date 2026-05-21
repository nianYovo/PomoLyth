#include "ai/TaskPlanner.h"

TaskPlanner::TaskPlanner(IAiClient& aiClient) : m_aiClient(aiClient) {}

FocusTask TaskPlanner::generatePlan(const QString& userInput, int minutes) {
    const QString response = m_aiClient.chat(m_promptBuilder.buildTaskPlanningPrompt(userInput));

    FocusTask task;
    task.originalInput = userInput.trimmed();
    task.estimatedMinutes = minutes;
    task.difficulty = "normal";
    task.goals = {
        "Define the smallest useful goal",
        "Finish the core step first",
        "Record one blocker for review"
    };
    task.avoidList = {
        "Avoid frequent context switching",
        "Avoid scope creep",
        "Avoid entertainment apps"
    };

    if (!response.trimmed().isEmpty()) {
        task.goals[0] = response.section("Avoid", 0, 0).trimmed().left(80);
    }

    return task;
}
