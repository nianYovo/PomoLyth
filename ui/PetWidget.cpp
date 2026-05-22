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
    painter.setPen(QPen(QColor("#F3C7D8"), 1.5));
    painter.setBrush(QColor(255, 255, 255, 236));
    painter.drawRoundedRect(bubbleRect, 16, 16);
    painter.setPen(QColor("#3A2630"));
    const QString speech = m_speech.isEmpty() ? QString("慢慢来，我陪你把这一轮完成。") : m_speech;
    painter.drawText(bubbleRect.adjusted(14, 8, -14, -8), Qt::TextWordWrap | Qt::AlignVCenter, speech);

    painter.setPen(QPen(QColor("#FF8DB3"), 2, Qt::SolidLine, Qt::RoundCap));
    painter.drawText(QRectF(width() - 54, 68, 38, 24), Qt::AlignCenter, "♥");

    const QPointF center(width() / 2.0, 142);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 141, 179, 38));
    painter.drawEllipse(QPointF(center.x() + 8, center.y() + 50), 62, 12);

    painter.setBrush(moodColor());
    painter.setPen(QPen(QColor("#3A2630"), 2));
    painter.drawEllipse(center, 62, 54);

    painter.setPen(QPen(QColor("#3A2630"), 2));
    painter.setBrush(moodColor().lighter(110));
    painter.drawEllipse(QPointF(center.x() - 48, center.y() - 42), 18, 22);
    painter.drawEllipse(QPointF(center.x() + 48, center.y() - 42), 18, 22);

    painter.setBrush(QColor("#3A2630"));
    painter.drawEllipse(QPointF(center.x() - 22, center.y() - 8), 5, 7);
    painter.drawEllipse(QPointF(center.x() + 22, center.y() - 8), 5, 7);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 141, 179, 125));
    painter.drawEllipse(QPointF(center.x() - 36, center.y() + 8), 10, 6);
    painter.drawEllipse(QPointF(center.x() + 36, center.y() + 8), 10, 6);

    QPainterPath bow;
    bow.moveTo(center.x() - 8, center.y() - 58);
    bow.lineTo(center.x() - 34, center.y() - 70);
    bow.lineTo(center.x() - 28, center.y() - 44);
    bow.closeSubpath();
    bow.moveTo(center.x() + 8, center.y() - 58);
    bow.lineTo(center.x() + 34, center.y() - 70);
    bow.lineTo(center.x() + 28, center.y() - 44);
    bow.closeSubpath();
    painter.setBrush(QColor("#FF8DB3"));
    painter.drawPath(bow);
    painter.setBrush(QColor("#E85F91"));
    painter.drawEllipse(QPointF(center.x(), center.y() - 57), 7, 7);

    painter.setPen(QPen(QColor("#3A2630"), 3, Qt::SolidLine, Qt::RoundCap));
    if (m_mood == PetMood::Celebrating || m_mood == PetMood::Proud) {
        painter.drawArc(QRectF(center.x() - 20, center.y() + 2, 40, 24), 200 * 16, 140 * 16);
    } else if (m_mood == PetMood::Angry || m_mood == PetMood::Worried) {
        painter.drawLine(QPointF(center.x() - 16, center.y() + 20), QPointF(center.x() + 16, center.y() + 20));
    } else {
        painter.drawArc(QRectF(center.x() - 14, center.y() + 4, 28, 16), 200 * 16, 140 * 16);
    }

    painter.setPen(QPen(QColor("#8F7482"), 2));
    painter.drawText(QRectF(0, height() - 30, width(), 24), Qt::AlignCenter, "🍅 PomoLyth");
}

QColor PetWidget::moodColor() const {
    switch (m_mood) {
    case PetMood::Idle: return QColor("#BFEAD4");
    case PetMood::Focused: return QColor("#EAFBF2");
    case PetMood::Encouraging: return QColor("#FFE7F0");
    case PetMood::Worried: return QColor("#FFF0C8");
    case PetMood::Angry: return QColor("#FFD4E3");
    case PetMood::Sleepy: return QColor("#F0E6FF");
    case PetMood::Proud: return QColor("#FFF9FC");
    case PetMood::Celebrating: return QColor("#BFEAD4");
    }
    return QColor("#BFEAD4");
}
