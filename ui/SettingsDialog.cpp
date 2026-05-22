#include "ui/SettingsDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSizePolicy>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>

static QFrame* settingsCard(const QString& title, QFormLayout** formOut) {
    auto* card = new QFrame;
    card->setObjectName("settingsCard");

    auto* titleLabel = new QLabel(title);
    titleLabel->setObjectName("sectionTitle");

    auto* form = new QFormLayout;
    form->setLabelAlignment(Qt::AlignLeft);
    form->setFormAlignment(Qt::AlignTop);
    form->setHorizontalSpacing(14);
    form->setVerticalSpacing(10);

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 14, 16, 16);
    layout->setSpacing(10);
    layout->addWidget(titleLabel);
    layout->addLayout(form);

    *formOut = form;
    return card;
}

static QFrame* listCard(const QString& title, QTextEdit* editor) {
    auto* card = new QFrame;
    card->setObjectName("settingsCard");

    auto* titleLabel = new QLabel(title);
    titleLabel->setObjectName("sectionTitle");
    auto* hint = new QLabel("每行一个关键词。");
    hint->setObjectName("subtleText");

    editor->setMinimumSize(220, 130);
    editor->setMaximumHeight(160);

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 14, 16, 16);
    layout->setSpacing(8);
    layout->addWidget(titleLabel);
    layout->addWidget(hint);
    layout->addWidget(editor);
    return card;
}

SettingsDialog::SettingsDialog(
    const AppConfig& config,
    const QStringList& blacklist,
    const QStringList& whitelist,
    QWidget* parent)
    : QDialog(parent) {
    setObjectName("settingsDialog");
    setWindowTitle("PomoLyth 设置");
    resize(720, 660);

    m_focusMinutes = new QSpinBox;
    m_focusMinutes->setRange(1, 180);
    m_focusMinutes->setValue(config.defaultFocusMinutes);
    m_focusMinutes->setSuffix(" min");
    m_focusMinutes->setFixedWidth(132);

    m_breakMinutes = new QSpinBox;
    m_breakMinutes->setRange(1, 60);
    m_breakMinutes->setValue(config.defaultBreakMinutes);
    m_breakMinutes->setSuffix(" min");
    m_breakMinutes->setFixedWidth(132);

    m_monitorInterval = new QSpinBox;
    m_monitorInterval->setRange(1, 60);
    m_monitorInterval->setValue(config.focusMonitorIntervalSeconds);
    m_monitorInterval->setSuffix(" s");
    m_monitorInterval->setFixedWidth(132);

    m_aiProviderBox = new QComboBox;
    m_aiProviderBox->addItems({"mock", "openai"});
    const int providerIndex = m_aiProviderBox->findText(config.aiProvider, Qt::MatchFixedString);
    m_aiProviderBox->setCurrentIndex(providerIndex >= 0 ? providerIndex : 0);
    m_aiProviderBox->setMaximumWidth(220);

    m_openAiBaseUrl = new QLineEdit(config.openAiBaseUrl);
    m_openAiBaseUrl->setPlaceholderText("https://api.openai.com/v1");
    m_openAiBaseUrl->setMaximumWidth(520);
    m_openAiModel = new QLineEdit(config.openAiModel);
    m_openAiModel->setPlaceholderText("gpt-4.1-mini");
    m_openAiModel->setMaximumWidth(320);
    m_openAiApiKeyEnv = new QLineEdit(config.openAiApiKeyEnv);
    m_openAiApiKeyEnv->setPlaceholderText("OPENAI_API_KEY");
    m_openAiApiKeyEnv->setMaximumWidth(320);

    m_blacklistEdit = new QTextEdit;
    m_blacklistEdit->setPlainText(blacklist.join('\n'));
    m_blacklistEdit->setPlaceholderText("每行一个关键词，例如 steam 或 bilibili。");
    m_blacklistEdit->setToolTip("每行一个关键词。");

    m_whitelistEdit = new QTextEdit;
    m_whitelistEdit->setPlainText(whitelist.join('\n'));
    m_whitelistEdit->setPlaceholderText("每行一个关键词。命中的窗口不会被判定为分心。");
    m_whitelistEdit->setToolTip("每行一个关键词。");

    auto* pomodoroCard = new QFrame;
    pomodoroCard->setObjectName("settingsCard");
    auto* pomodoroTitle = new QLabel("计时器和监控设置");
    pomodoroTitle->setObjectName("sectionTitle");
    auto* pomodoroGrid = new QGridLayout;
    pomodoroGrid->setHorizontalSpacing(14);
    pomodoroGrid->setVerticalSpacing(10);
    pomodoroGrid->addWidget(new QLabel("专注时长"), 0, 0);
    pomodoroGrid->addWidget(m_focusMinutes, 0, 1);
    pomodoroGrid->addWidget(new QLabel("休息时长"), 0, 2);
    pomodoroGrid->addWidget(m_breakMinutes, 0, 3);
    pomodoroGrid->addWidget(new QLabel("检测间隔"), 0, 4);
    pomodoroGrid->addWidget(m_monitorInterval, 0, 5);
    pomodoroGrid->setColumnStretch(6, 1);
    auto* pomodoroLayout = new QVBoxLayout(pomodoroCard);
    pomodoroLayout->setContentsMargins(16, 14, 16, 16);
    pomodoroLayout->setSpacing(10);
    pomodoroLayout->addWidget(pomodoroTitle);
    pomodoroLayout->addLayout(pomodoroGrid);

    QFormLayout* aiForm = nullptr;
    auto* aiCard = settingsCard("AI 设置", &aiForm);
    aiForm->addRow("AI Provider", m_aiProviderBox);
    aiForm->addRow("OpenAI 地址", m_openAiBaseUrl);
    aiForm->addRow("OpenAI 模型", m_openAiModel);
    aiForm->addRow("API Key 环境变量", m_openAiApiKeyEnv);

    auto* blacklistCard = listCard("分心黑名单", m_blacklistEdit);
    auto* whitelistCard = listCard("学习白名单", m_whitelistEdit);
    auto* listRow = new QHBoxLayout;
    listRow->setContentsMargins(0, 0, 0, 0);
    listRow->setSpacing(14);
    listRow->addWidget(blacklistCard);
    listRow->addWidget(whitelistCard);

    auto* note = new QLabel("计时器和监控设置会立即生效。AI Provider 修改后需要重启应用。");
    note->setObjectName("statusLabel");
    note->setWordWrap(true);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText("保存设置");
    buttons->button(QDialogButtonBox::Ok)->setProperty("buttonRole", "primary");
    buttons->button(QDialogButtonBox::Ok)->setFixedWidth(112);
    buttons->button(QDialogButtonBox::Cancel)->setText("取消");
    buttons->button(QDialogButtonBox::Cancel)->setProperty("buttonRole", "outline");
    buttons->button(QDialogButtonBox::Cancel)->setFixedWidth(112);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(14);
    layout->addWidget(note);
    layout->addWidget(pomodoroCard);
    layout->addWidget(aiCard);
    auto* listTitle = new QLabel("名单管理");
    listTitle->setObjectName("sectionTitle");
    layout->addWidget(listTitle);
    layout->addLayout(listRow, 1);
    layout->addWidget(buttons);
}

AppConfig SettingsDialog::appConfig() const {
    AppConfig config;
    config.defaultFocusMinutes = m_focusMinutes->value();
    config.defaultBreakMinutes = m_breakMinutes->value();
    config.focusMonitorIntervalSeconds = m_monitorInterval->value();
    config.aiProvider = m_aiProviderBox->currentText();
    config.openAiBaseUrl = m_openAiBaseUrl->text().trimmed();
    config.openAiModel = m_openAiModel->text().trimmed();
    config.openAiApiKeyEnv = m_openAiApiKeyEnv->text().trimmed();
    return config;
}

QStringList SettingsDialog::blacklist() const {
    return linesFromText(m_blacklistEdit->toPlainText());
}

QStringList SettingsDialog::whitelist() const {
    return linesFromText(m_whitelistEdit->toPlainText());
}

QStringList SettingsDialog::linesFromText(const QString& text) {
    QStringList lines;
    for (const QString& line : text.split('\n')) {
        const QString trimmed = line.trimmed();
        if (!trimmed.isEmpty() && !lines.contains(trimmed, Qt::CaseInsensitive)) {
            lines.append(trimmed);
        }
    }
    return lines;
}
