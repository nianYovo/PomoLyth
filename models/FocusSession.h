#pragma once

#include <QDateTime>
#include <QString>
#include <QStringList>

struct FocusSession {
    int id = -1;
    QString task;
    int plannedMinutes = 25;
    int actualMinutes = 0;
    int distractionCount = 0;
    QStringList completedGoals;
    QStringList problems;
    QString aiSummary;
    QString nextSuggestion;
    QDateTime createdAt = QDateTime::currentDateTime();
};
