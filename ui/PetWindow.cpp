#include "ui/PetWindow.h"

#include <QMouseEvent>
#include <QVBoxLayout>
#include "ui/PetWidget.h"

PetWindow::PetWindow(QWidget* parent) : QWidget(parent) {
    setWindowTitle("PomoLyth Pet");
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(280, 240);

    m_petWidget = new PetWidget;
    m_petWidget->setAttribute(Qt::WA_TranslucentBackground);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_petWidget);
}

void PetWindow::setMood(PetMood mood, const QString& speech) {
    m_petWidget->setMood(mood, speech);
}

void PetWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragOffset = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void PetWindow::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - m_dragOffset);
        event->accept();
    }
}
