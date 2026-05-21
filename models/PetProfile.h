#pragma once

#include <QStringList>

struct PetProfile {
    int level = 1;
    int exp = 0;
    int intimacy = 0;
    int energy = 100;
    int totalFocusMinutes = 0;
    int completedPomodoros = 0;
    int continuousDays = 0;
    QStringList achievements;
};
