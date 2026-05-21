#pragma once

#include <QObject>
#include <QStringList>
#include "models/FocusSession.h"
#include "models/PetProfile.h"

class AchievementSystem : public QObject {
    Q_OBJECT

public:
    explicit AchievementSystem(QObject* parent = nullptr);

    QStringList update(const FocusSession& session, PetProfile& pet);
    QStringList lastUnlocked() const;

private:
    void unlock(PetProfile& pet, const QString& achievement);

    QStringList m_lastUnlocked;
};
