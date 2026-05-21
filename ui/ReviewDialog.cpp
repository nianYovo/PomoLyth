#include "ui/ReviewDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>

static QStringList linesFromText(const QString& text) {
    QStringList lines;
    for (const QString& line : text.split('\n')) {
        const QString trimmed = line.trimmed();
        if (!trimmed.isEmpty()) {
            lines.append(trimmed);
        }
    }
    return lines;
}

ReviewDialog::ReviewDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("本轮复盘");
    resize(460, 360);

    m_completedEdit = new QTextEdit;
    m_completedEdit->setPlaceholderText("这一轮完成了什么？可以一行一个。");
    m_problemsEdit = new QTextEdit;
    m_problemsEdit->setPlaceholderText("哪里卡住了？下一轮需要注意什么？");
    m_ratingSpin = new QSpinBox;
    m_ratingSpin->setRange(1, 5);
    m_ratingSpin->setValue(4);

    auto* form = new QFormLayout;
    form->addRow("完成内容", m_completedEdit);
    form->addRow("问题记录", m_problemsEdit);
    form->addRow("自评分", m_ratingSpin);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

QStringList ReviewDialog::completedGoals() const {
    return linesFromText(m_completedEdit->toPlainText());
}

QStringList ReviewDialog::problems() const {
    return linesFromText(m_problemsEdit->toPlainText());
}

int ReviewDialog::selfRating() const {
    return m_ratingSpin->value();
}
