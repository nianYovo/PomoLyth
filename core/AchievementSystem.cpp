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
        unlock(pet, "初次专注");
    }
    if (pet.completedPomodoros == 3) {
        unlock(pet, "三连番茄");
    }
    if (session.distractionCount == 0) {
        unlock(pet, "分心克星");
    }
    if (pet.totalFocusMinutes >= 120) {
        unlock(pet, "今日很强");
    }
    if (pet.level > oldLevel) {
        unlock(pet, QString("Lv.%1 升级").arg(pet.level));
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
