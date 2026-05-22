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
        m_dragging = true;
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
        m_dragging = false;
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
    QAction* startAction = menu.addAction(QString::fromUtf8("开始/写计划"));
    QAction* pauseAction = menu.addAction(QString::fromUtf8("暂停"));
    QAction* resumeAction = menu.addAction(QString::fromUtf8("继续"));
    QAction* stopAction = menu.addAction(QString::fromUtf8("结束"));
    menu.addSeparator();
    QAction* calendarAction = menu.addAction(QString::fromUtf8("显示月历"));
    QAction* hideAction = menu.addAction(QString::fromUtf8("隐藏桌宠"));

    QAction* selected = menu.exec(event->globalPos());
    if (selected == startAction) {
        emit startRequested();
    } else if (selected == pauseAction) {
        emit pauseRequested();
    } else if (selected == resumeAction) {
        emit resumeRequested();
    } else if (selected == stopAction) {
        emit stopRequested();
    } else if (selected == calendarAction) {
        emit calendarRequested();
    } else if (selected == hideAction) {
        hide();
    }
}
