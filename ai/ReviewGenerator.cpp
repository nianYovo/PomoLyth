#include "ai/ReviewGenerator.h"

ReviewGenerator::ReviewGenerator(IAiClient& aiClient) : m_aiClient(aiClient) {}

FocusSession ReviewGenerator::generateReview(FocusSession session) {
    const QString response = m_aiClient.chat(m_promptBuilder.buildReviewPrompt(session));
    session.aiSummary = response.trimmed().isEmpty()
        ? "本轮已经完成，建议下一轮继续保持任务足够小、目标足够明确。"
        : response.trimmed();
    session.nextSuggestion = "下一轮先处理本轮记录的卡点，再继续扩展。";
    return session;
}

QString ReviewGenerator::generateDailyReport(const QVector<FocusSession>& sessions) {
    const QString response = m_aiClient.chat(m_promptBuilder.buildDailyReportPrompt(sessions));
    if (!response.trimmed().isEmpty()) {
        return response.trimmed();
    }
    return "今日已有专注记录。建议明天继续保持小目标、短反馈的节奏。";
}

QString ReviewGenerator::generateDailyReport(const QVector<FocusSession>& sessions, const QString& memorySummary) {
    const QString response = m_aiClient.chat(m_promptBuilder.buildDailyReportPrompt(sessions, memorySummary));
    if (!response.trimmed().isEmpty()) {
        return response.trimmed();
    }
    return QString("今日已有专注记录。%1").arg(memorySummary);
}
