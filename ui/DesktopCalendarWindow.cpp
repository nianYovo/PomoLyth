#include "ui/DesktopCalendarWindow.h"

#include <QAbstractItemView>
#include <QCalendarWidget>
#include <QCursor>
#include <QEvent>
#include <QLabel>
#include <QListWidget>
#include <QMouseEvent>
#include <QTextCharFormat>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

DesktopCalendarWindow::DesktopCalendarWindow(SQLiteStorage& storage, QWidget* parent)
    : QWidget(parent),
      m_storage(storage) {
    setWindowTitle("PomoLyth Desktop Calendar");
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumSize(360, 460);
    resize(480, 620);
    setWindowOpacity(0.82);
    buildUi();
    loadSelectedDate();
    refreshMarks();
    refreshTodoItems();
}

void DesktopCalendarWindow::buildUi() {
    setObjectName("desktopCalendar");
    setStyleSheet(R"(
        #desktopCalendar {
            background: rgba(255, 249, 252, 176);
            border: 1px solid rgba(255, 141, 179, 120);
            border-radius: 14px;
        }
        QLabel#calendarTitle {
            color: #6D2A46;
            font-size: 16px;
            font-weight: 800;
            padding: 4px 6px;
        }
        QLabel#selectedDateLabel {
            color: #6D2A46;
            font-size: 13px;
            font-weight: 700;
            padding: 2px 4px;
        }
        QCalendarWidget {
            background: rgba(255, 255, 255, 110);
            color: #3A2630;
            border: 1px solid rgba(243, 199, 216, 150);
            border-radius: 10px;
        }
        QCalendarWidget QToolButton {
            color: #6D2A46;
            background: transparent;
            font-weight: 700;
            min-height: 28px;
        }
        QCalendarWidget QAbstractItemView {
            selection-background-color: rgba(255, 141, 179, 170);
            selection-color: #3A2630;
            background: rgba(255, 255, 255, 80);
            outline: 0;
        }
        QListWidget {
            background: rgba(255, 255, 255, 104);
            border: 1px solid rgba(243, 199, 216, 140);
            border-radius: 10px;
            color: #3A2630;
            padding: 4px;
            font-size: 12px;
        }
        QListWidget::item {
            border-radius: 7px;
            padding: 5px 7px;
            min-height: 20px;
        }
        QListWidget::item:selected {
            background: rgba(255, 141, 179, 150);
            color: #3A2630;
        }
        QTextEdit {
            background: rgba(255, 255, 255, 132);
            border: 1px solid rgba(243, 199, 216, 150);
            border-radius: 10px;
            color: #3A2630;
            padding: 8px;
            font-size: 13px;
        }
    )");

    m_title = new QLabel(QString::fromUtf8("月历待办"));
    m_title->setObjectName("calendarTitle");
    m_title->setCursor(Qt::OpenHandCursor);
    m_title->installEventFilter(this);

    m_calendar = new QCalendarWidget;
    m_calendar->setGridVisible(false);
    m_calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    m_calendar->setSelectedDate(m_selectedDate);

    m_todoItems = new QListWidget;
    m_todoItems->setMinimumHeight(76);
    m_todoItems->setMaximumHeight(116);
    m_todoItems->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    m_selectedDateLabel = new QLabel;
    m_selectedDateLabel->setObjectName("selectedDateLabel");
    m_selectedDateLabel->setCursor(Qt::OpenHandCursor);
    m_selectedDateLabel->installEventFilter(this);

    m_notes = new QTextEdit;
    m_notes->setPlaceholderText(QString::fromUtf8("写下这一天要做的事情..."));
    m_notes->setMinimumHeight(130);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 12, 14, 14);
    layout->setSpacing(10);
    layout->addWidget(m_title);
    layout->addWidget(m_calendar, 1);
    layout->addWidget(m_todoItems, 0);
    layout->addWidget(m_selectedDateLabel);
    layout->addWidget(m_notes, 0);

    connect(m_calendar, &QCalendarWidget::selectionChanged, this, [this]() {
        saveSelectedDate();
        m_selectedDate = m_calendar->selectedDate();
        loadSelectedDate();
        refreshTodoItems();
    });
    connect(m_calendar, &QCalendarWidget::currentPageChanged, this, [this](int, int) {
        refreshMarks();
        refreshTodoItems();
    });
    connect(m_todoItems, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        const QDate date = item->data(Qt::UserRole).toDate();
        if (!date.isValid()) {
            return;
        }
        saveSelectedDate();
        m_selectedDate = date;
        m_calendar->setSelectedDate(date);
        loadSelectedDate();
        refreshTodoItems();
    });
    connect(m_notes, &QTextEdit::textChanged, this, [this]() {
        if (!m_loading) {
            saveSelectedDate();
            refreshMarks();
            refreshTodoItems();
        }
    });
}

void DesktopCalendarWindow::loadSelectedDate() {
    m_loading = true;
    m_notes->setPlainText(m_storage.dayPlan(m_selectedDate));
    m_selectedDateLabel->setText(QString::fromUtf8("%1 的待办").arg(m_selectedDate.toString("yyyy-MM-dd")));
    m_loading = false;
}

void DesktopCalendarWindow::saveSelectedDate() {
    if (m_selectedDate.isValid()) {
        m_storage.saveDayPlan(m_selectedDate, m_notes->toPlainText());
    }
}

void DesktopCalendarWindow::refreshMarks() {
    const int year = m_calendar->yearShown();
    const int month = m_calendar->monthShown();
    m_monthPlans = m_storage.monthPlans(year, month);

    QTextCharFormat clearFormat;
    for (int day = 1; day <= QDate(year, month, 1).daysInMonth(); ++day) {
        m_calendar->setDateTextFormat(QDate(year, month, day), clearFormat);
    }

    QTextCharFormat format;
    format.setForeground(QColor("#6D2A46"));
    format.setBackground(QColor(255, 231, 240, 150));
    format.setFontWeight(QFont::DemiBold);
    for (auto it = m_monthPlans.cbegin(); it != m_monthPlans.cend(); ++it) {
        if (!it.value().trimmed().isEmpty()) {
            m_calendar->setDateTextFormat(it.key(), format);
        }
    }
}

void DesktopCalendarWindow::refreshTodoItems() {
    m_todoItems->clear();
    const int year = m_calendar->yearShown();
    const int month = m_calendar->monthShown();
    m_monthPlans = m_storage.monthPlans(year, month);

    for (auto it = m_monthPlans.cbegin(); it != m_monthPlans.cend(); ++it) {
        const QString content = it.value().simplified();
        if (content.isEmpty()) {
            continue;
        }

        QString preview = content;
        if (preview.size() > 28) {
            preview = preview.left(28) + "...";
        }

        auto* item = new QListWidgetItem(QString("%1  %2").arg(it.key().toString("MM-dd"), preview));
        item->setData(Qt::UserRole, it.key());
        m_todoItems->addItem(item);
        if (it.key() == m_selectedDate) {
            item->setSelected(true);
            m_todoItems->setCurrentItem(item);
        }
    }

    if (m_todoItems->count() == 0) {
        auto* item = new QListWidgetItem(QString::fromUtf8("本月还没有写待办"));
        item->setFlags(Qt::NoItemFlags);
        m_todoItems->addItem(item);
    }
}

void DesktopCalendarWindow::keepAtDesktopBottom() {
#ifdef Q_OS_WIN
    HWND hwnd = reinterpret_cast<HWND>(winId());
    SetWindowPos(hwnd, HWND_BOTTOM, x(), y(), width(), height(), SWP_NOACTIVATE | SWP_SHOWWINDOW);
#else
    lower();
#endif
}

bool DesktopCalendarWindow::isResizeHotspot(const QPoint& position) const {
    constexpr int grip = 22;
    return position.x() >= width() - grip && position.y() >= height() - grip;
}

bool DesktopCalendarWindow::beginMoveFromEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton || isResizeHotspot(event->position().toPoint())) {
        return false;
    }

    m_dragMode = DragMode::Move;
    m_dragOffset = event->globalPosition().toPoint() - frameGeometry().topLeft();
    m_title->setCursor(Qt::ClosedHandCursor);
    grabMouse();
    event->accept();
    return true;
}

void DesktopCalendarWindow::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    keepAtDesktopBottom();
}

void DesktopCalendarWindow::changeEvent(QEvent* event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::ActivationChange || event->type() == QEvent::WindowStateChange) {
        QTimer::singleShot(0, this, &DesktopCalendarWindow::keepAtDesktopBottom);
    }
}

bool DesktopCalendarWindow::eventFilter(QObject* watched, QEvent* event) {
    if ((watched == m_title || watched == m_selectedDateLabel) && event->type() == QEvent::MouseButtonPress) {
        return beginMoveFromEvent(static_cast<QMouseEvent*>(event));
    }
    return QWidget::eventFilter(watched, event);
}

void DesktopCalendarWindow::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    setCursor(isResizeHotspot(mapFromGlobal(QCursor::pos())) ? Qt::SizeFDiagCursor : Qt::ArrowCursor);
    keepAtDesktopBottom();
}

void DesktopCalendarWindow::mousePressEvent(QMouseEvent* event) {
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }

    if (isResizeHotspot(event->position().toPoint())) {
        m_dragMode = DragMode::Resize;
        m_resizeStartSize = size();
        m_resizeStartGlobal = event->globalPosition().toPoint();
        event->accept();
        return;
    }

    const QWidget* child = childAt(event->position().toPoint());
    const bool onInteractiveChild = child == m_calendar || child == m_notes || child == m_todoItems
        || (child && (m_calendar->isAncestorOf(child) || m_notes->isAncestorOf(child) || m_todoItems->isAncestorOf(child)));
    if (!onInteractiveChild && beginMoveFromEvent(event)) {
        return;
    }

    QWidget::mousePressEvent(event);
}

void DesktopCalendarWindow::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragMode == DragMode::Resize) {
        const QPoint delta = event->globalPosition().toPoint() - m_resizeStartGlobal;
        resize((m_resizeStartSize + QSize(delta.x(), delta.y())).expandedTo(minimumSize()));
        keepAtDesktopBottom();
        event->accept();
        return;
    }

    if (m_dragMode == DragMode::Move) {
        move(event->globalPosition().toPoint() - m_dragOffset);
        keepAtDesktopBottom();
        event->accept();
        return;
    }

    setCursor(isResizeHotspot(event->position().toPoint()) ? Qt::SizeFDiagCursor : Qt::ArrowCursor);
    QWidget::mouseMoveEvent(event);
}

void DesktopCalendarWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragMode = DragMode::None;
        m_title->setCursor(Qt::OpenHandCursor);
        releaseMouse();
        keepAtDesktopBottom();
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}
