#include "ui/PetWindow.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QVBoxLayout>
#include "ui/PetWidget.h"

PetWindow::PetWindow(QWidget* parent) : QWidget(parent) {
    setWindowTitle("PomoLyth Pet");
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool | Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoFillBackground(false);
    setCursor(Qt::OpenHandCursor);
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
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
}

void PetWindow::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - m_dragOffset);
        event->accept();
    }
}

void PetWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        setCursor(Qt::OpenHandCursor);
        event->accept();
    }
}

void PetWindow::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        hide();
        event->accept();
    }
}

void PetWindow::contextMenuEvent(QContextMenuEvent* event) {
    QMenu menu(this);
    QAction* hideAction = menu.addAction("隐藏桌宠");
    QAction* selected = menu.exec(event->globalPos());
    if (selected == hideAction) {
        hide();
    }
}
