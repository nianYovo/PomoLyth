#include "ui/PetWidget.h"

#include <QPainter>
#include <QPainterPath>

PetWidget::PetWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(260, 220);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);
}

void PetWidget::setMood(PetMood mood, const QString& speech) {
    m_mood = mood;
    m_speech = speech;
    update();
}

void PetWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    const QRectF bubbleRect(12, 10, width() - 24, 64);
    painter.setPen(QPen(QColor(205, 213, 199, 210), 1));
    painter.setBrush(QColor(255, 255, 250, 225));
    painter.drawRoundedRect(bubbleRect, 8, 8);
    painter.setPen(QColor("#26302a"));
    painter.drawText(bubbleRect.adjusted(12, 8, -12, -8), Qt::TextWordWrap | Qt::AlignVCenter, m_speech);

    const QPointF center(width() / 2.0, 142);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(38, 48, 42, 35));
    painter.drawEllipse(QPointF(center.x() + 8, center.y() + 50), 62, 12);

    painter.setBrush(moodColor());
    painter.setPen(QPen(QColor("#26302a"), 2));
    painter.drawEllipse(center, 62, 54);

    painter.setPen(QPen(QColor("#26302a"), 2));
    painter.setBrush(moodColor().lighter(110));
    painter.drawEllipse(QPointF(center.x() - 48, center.y() - 42), 18, 22);
    painter.drawEllipse(QPointF(center.x() + 48, center.y() - 42), 18, 22);

    painter.setBrush(QColor("#26302a"));
    painter.drawEllipse(QPointF(center.x() - 22, center.y() - 8), 5, 7);
    painter.drawEllipse(QPointF(center.x() + 22, center.y() - 8), 5, 7);

    painter.setPen(QPen(QColor("#26302a"), 3, Qt::SolidLine, Qt::RoundCap));
    if (m_mood == PetMood::Celebrating || m_mood == PetMood::Proud) {
        painter.drawArc(QRectF(center.x() - 20, center.y() + 2, 40, 24), 200 * 16, 140 * 16);
    } else if (m_mood == PetMood::Angry || m_mood == PetMood::Worried) {
        painter.drawLine(QPointF(center.x() - 16, center.y() + 20), QPointF(center.x() + 16, center.y() + 20));
    } else {
        painter.drawArc(QRectF(center.x() - 14, center.y() + 4, 28, 16), 200 * 16, 140 * 16);
    }

    painter.setPen(QPen(QColor("#6c7b68"), 2));
    painter.drawText(QRectF(0, height() - 30, width(), 24), Qt::AlignCenter, "PomoLyth");
}

QColor PetWidget::moodColor() const {
    switch (m_mood) {
    case PetMood::Idle: return QColor("#d9ead3");
    case PetMood::Focused: return QColor("#b8d8ff");
    case PetMood::Encouraging: return QColor("#fce5cd");
    case PetMood::Worried: return QColor("#ffe599");
    case PetMood::Angry: return QColor("#f4cccc");
    case PetMood::Sleepy: return QColor("#d9d2e9");
    case PetMood::Proud: return QColor("#c9daf8");
    case PetMood::Celebrating: return QColor("#b6d7a8");
    }
    return QColor("#d9ead3");
}
