#include "core/AchievementSystem.h"

AchievementSystem::AchievementSystem(QObject* parent) : QObject(parent) {}

QStringList AchievementSystem::update(const FocusSession& session, PetProfile& pet) {
    m_lastUnlocked.clear();

    const int baseExp = session.distractionCount == 0 ? 15 : 10;
    pet.exp += baseExp;
    pet.intimacy += session.distractionCount == 0 ? 3 : 1;
    pet.totalFocusMinutes += session.actualMinutes;
    pet.completedPomodoros += 1;

    const int oldLevel = pet.level;
    pet.level = 1 + pet.exp / 100;

    if (pet.completedPomodoros == 1) {
        unlock(pet, "First Focus");
    }
    if (pet.completedPomodoros == 3) {
        unlock(pet, "Triple Pomodoro");
    }
    if (session.distractionCount == 0) {
        unlock(pet, "Distraction Free");
    }
    if (pet.totalFocusMinutes >= 120) {
        unlock(pet, "Strong Day");
    }
    if (pet.level > oldLevel) {
        unlock(pet, QString("Level %1").arg(pet.level));
    }

    return m_lastUnlocked;
}

QStringList AchievementSystem::lastUnlocked() const {
    return m_lastUnlocked;
}

void AchievementSystem::unlock(PetProfile& pet, const QString& achievement) {
    if (!pet.achievements.contains(achievement)) {
        pet.achievements.append(achievement);
        m_lastUnlocked.append(achievement);
    }
}
