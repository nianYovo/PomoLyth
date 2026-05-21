#pragma once

#include <QString>
#include <QVector>
#include "models/FocusSession.h"

class PromptBuilder {
public:
    QString buildTaskPlanningPrompt(const QString& userInput) const;
    QString buildReviewPrompt(const FocusSession& session) const;
    QString buildDailyReportPrompt(const QVector<FocusSession>& sessions) const;
    QString buildDailyReportPrompt(const QVector<FocusSession>& sessions, const QString& memorySummary) const;
};
