#pragma once

#include <QString>
#include <QStringList>
#include "storage/SQLiteStorage.h"

struct UserMemorySnapshot {
    int totalFocusDays = 0;
    int totalFocusMinutes = 0;
    int bestFocusHour = -1;
    QString commonDistractionSource;
    QStringList frequentTasks;
    QString summary;
};

class UserMemory {
public:
    explicit UserMemory(SQLiteStorage& storage);

    UserMemorySnapshot buildSnapshot() const;

private:
    SQLiteStorage& m_storage;
};
