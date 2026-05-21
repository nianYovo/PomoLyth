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
    setWindowTitle("Session Review");
    resize(460, 360);

    m_completedEdit = new QTextEdit;
    m_completedEdit->setPlaceholderText("What did you complete? One item per line is fine.");
    m_problemsEdit = new QTextEdit;
    m_problemsEdit->setPlaceholderText("What got stuck? What should the next round watch for?");
    m_ratingSpin = new QSpinBox;
    m_ratingSpin->setRange(1, 5);
    m_ratingSpin->setValue(4);

    auto* form = new QFormLayout;
    form->addRow("Completed", m_completedEdit);
    form->addRow("Problems", m_problemsEdit);
    form->addRow("Rating", m_ratingSpin);

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
