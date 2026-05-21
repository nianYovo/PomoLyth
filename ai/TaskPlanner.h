#pragma once

#include "ai/IAiClient.h"
#include "ai/PromptBuilder.h"
#include "models/FocusTask.h"

class TaskPlanner {
public:
    explicit TaskPlanner(IAiClient& aiClient);
    FocusTask generatePlan(const QString& userInput, int minutes = 25);

private:
    IAiClient& m_aiClient;
    PromptBuilder m_promptBuilder;
};
