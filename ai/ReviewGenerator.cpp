#include "ai/ReviewGenerator.h"

ReviewGenerator::ReviewGenerator(IAiClient& aiClient) : m_aiClient(aiClient) {}

FocusSession ReviewGenerator::generateReview(FocusSession session) {
    const QString response = m_aiClient.chat(m_promptBuilder.buildReviewPrompt(session));
    session.aiSummary = response.trimmed().isEmpty()
        ? "Session completed. Keep the next task small and concrete."
        : response.trimmed();
    session.nextSuggestion = "Handle the recorded blocker first, then expand.";
    return session;
}

QString ReviewGenerator::generateDailyReport(const QVector<FocusSession>& sessions) {
    const QString response = m_aiClient.chat(m_promptBuilder.buildDailyReportPrompt(sessions));
    if (!response.trimmed().isEmpty()) {
        return response.trimmed();
    }
    return "Focus records exist today. Keep tomorrow's goals small and review quickly.";
}

QString ReviewGenerator::generateDailyReport(const QVector<FocusSession>& sessions, const QString& memorySummary) {
    const QString response = m_aiClient.chat(m_promptBuilder.buildDailyReportPrompt(sessions, memorySummary));
    if (!response.trimmed().isEmpty()) {
        return response.trimmed();
    }
    return QString("Focus records exist today. %1").arg(memorySummary);
}
