#pragma once

#include <QDateTime>
#include <QString>

struct DistractionEvent {
    QString windowTitle;
    QString processName;
    QDateTime timestamp = QDateTime::currentDateTime();
    int severity = 1;
};
