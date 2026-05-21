#include "ui/SettingsDialog.h"

#include <QDialogButtonBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(
    const AppConfig& config,
    const QStringList& blacklist,
    const QStringList& whitelist,
    QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("PomoLyth 设置");
    resize(520, 520);

    m_focusMinutes = new QSpinBox;
    m_focusMinutes->setRange(1, 180);
    m_focusMinutes->setValue(config.defaultFocusMinutes);
    m_focusMinutes->setSuffix(" min");

    m_breakMinutes = new QSpinBox;
    m_breakMinutes->setRange(1, 60);
    m_breakMinutes->setValue(config.defaultBreakMinutes);
    m_breakMinutes->setSuffix(" min");

    m_monitorInterval = new QSpinBox;
    m_monitorInterval->setRange(1, 60);
    m_monitorInterval->setValue(config.focusMonitorIntervalSeconds);
    m_monitorInterval->setSuffix(" s");

    m_aiProviderBox = new QComboBox;
    m_aiProviderBox->addItems({"mock", "openai"});
    const int providerIndex = m_aiProviderBox->findText(config.aiProvider, Qt::MatchFixedString);
    m_aiProviderBox->setCurrentIndex(providerIndex >= 0 ? providerIndex : 0);

    m_openAiBaseUrl = new QLineEdit(config.openAiBaseUrl);
    m_openAiBaseUrl->setPlaceholderText("https://api.openai.com/v1");
    m_openAiModel = new QLineEdit(config.openAiModel);
    m_openAiModel->setPlaceholderText("gpt-4.1-mini");
    m_openAiApiKeyEnv = new QLineEdit(config.openAiApiKeyEnv);
    m_openAiApiKeyEnv->setPlaceholderText("OPENAI_API_KEY");

    m_blacklistEdit = new QTextEdit;
    m_blacklistEdit->setPlainText(blacklist.join('\n'));
    m_blacklistEdit->setPlaceholderText("一行一个关键词，例如 steam 或 bilibili。");

    m_whitelistEdit = new QTextEdit;
    m_whitelistEdit->setPlainText(whitelist.join('\n'));
    m_whitelistEdit->setPlaceholderText("一行一个关键词。命中的窗口不会被判定为分心。");

    auto* form = new QFormLayout;
    form->addRow("专注时长", m_focusMinutes);
    form->addRow("休息时长", m_breakMinutes);
    form->addRow("检测间隔", m_monitorInterval);
    form->addRow("AI Provider", m_aiProviderBox);
    form->addRow("OpenAI 地址", m_openAiBaseUrl);
    form->addRow("OpenAI 模型", m_openAiModel);
    form->addRow("API Key 环境变量", m_openAiApiKeyEnv);
    form->addRow("分心黑名单", m_blacklistEdit);
    form->addRow("学习白名单", m_whitelistEdit);

    auto* note = new QLabel("计时器和监控设置会立即生效。AI Provider 修改后需要重启应用。");
    note->setWordWrap(true);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(note);
    layout->addLayout(form);
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
