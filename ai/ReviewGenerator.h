#pragma once

#include "ai/IAiClient.h"
#include "ai/PromptBuilder.h"
#include "models/FocusSession.h"

class ReviewGenerator {
public:
    explicit ReviewGenerator(IAiClient& aiClient);
    FocusSession generateReview(FocusSession session);
    QString generateDailyReport(const QVector<FocusSession>& sessions);
    QString generateDailyReport(const QVector<FocusSession>& sessions, const QString& memorySummary);

private:
    IAiClient& m_aiClient;
    PromptBuilder m_promptBuilder;
};
