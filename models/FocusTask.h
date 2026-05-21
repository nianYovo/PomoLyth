#pragma once

#include <QString>
#include <QStringList>

struct FocusTask {
    QString originalInput;
    QStringList goals;
    QStringList avoidList;
    int estimatedMinutes = 25;
    QString difficulty = "normal";
};
