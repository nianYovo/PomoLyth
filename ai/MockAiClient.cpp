#include "ai/MockAiClient.h"

QString MockAiClient::chat(const QString& prompt) {
    if (prompt.contains("review", Qt::CaseInsensitive)) {
        return "Session review: you completed a clear focus step. Next time, keep the goal small and isolate the blocker.";
    }

    if (prompt.contains("daily", Qt::CaseInsensitive)) {
        return "Daily focus report: useful focus records were captured today. Tomorrow, schedule the hardest task first and close common distraction windows early.";
    }

    return "Plan: 1. Define the smallest useful goal; 2. Finish the core step first; 3. Record one blocker. Avoid: frequent context switching and scope creep.";
}
