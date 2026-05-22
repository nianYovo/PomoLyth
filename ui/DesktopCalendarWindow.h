#pragma once

#include <QDate>
#include <QMap>
#include <QPoint>
#include <QWidget>
#include "storage/SQLiteStorage.h"

class QCalendarWidget;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QTextEdit;

class DesktopCalendarWindow : public QWidget {
    Q_OBJECT

public:
    explicit DesktopCalendarWindow(SQLiteStorage& storage, QWidget* parent = nullptr);

protected:
    void changeEvent(QEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    enum class DragMode {
        None,
        Move,
        Resize
    };

    void buildUi();
    void loadSelectedDate();
    void saveSelectedDate();
    void refreshMarks();
    void refreshTodoItems();
    void keepAtDesktopBottom();
    bool isResizeHotspot(const QPoint& position) const;
    bool beginMoveFromEvent(QMouseEvent* event);

    SQLiteStorage& m_storage;
    QCalendarWidget* m_calendar = nullptr;
    QListWidget* m_todoItems = nullptr;
    QTextEdit* m_notes = nullptr;
    QLabel* m_title = nullptr;
    QLabel* m_selectedDateLabel = nullptr;
    QDate m_selectedDate = QDate::currentDate();
    QMap<QDate, QString> m_monthPlans;
    DragMode m_dragMode = DragMode::None;
    QPoint m_dragOffset;
    QSize m_resizeStartSize;
    QPoint m_resizeStartGlobal;
    bool m_loading = false;
};
