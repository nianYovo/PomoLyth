#pragma once

#include <QPoint>
#include <QWidget>
#include "core/PetStateMachine.h"

class PetWidget;

class PetWindow : public QWidget {
    Q_OBJECT

public:
    explicit PetWindow(QWidget* parent = nullptr);

public slots:
    void setMood(PetMood mood, const QString& speech);

signals:
    void startRequested();
    void pauseRequested();
    void resumeRequested();
    void stopRequested();
    void calendarRequested();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    PetWidget* m_petWidget = nullptr;
    QPoint m_dragOffset;
    bool m_dragging = false;
};
