#pragma once

#include <QWidget>
#include "core/PetStateMachine.h"

class QLabel;

class PetWidget : public QWidget {
    Q_OBJECT

public:
    explicit PetWidget(QWidget* parent = nullptr);

public slots:
    void setMood(PetMood mood, const QString& speech);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QColor moodColor() const;

    PetMood m_mood = PetMood::Idle;
    QString m_speech = "准备好开始一轮专注了吗？";
};
