#pragma once

#include <QDialog>

class QLineEdit;
class QSpinBox;
class QTextEdit;

class ReviewDialog : public QDialog {
    Q_OBJECT

public:
    explicit ReviewDialog(QWidget* parent = nullptr);

    QStringList completedGoals() const;
    QStringList problems() const;
    int selfRating() const;

private:
    QTextEdit* m_completedEdit = nullptr;
    QTextEdit* m_problemsEdit = nullptr;
    QSpinBox* m_ratingSpin = nullptr;
};
