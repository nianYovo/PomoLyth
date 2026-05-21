#pragma once

#include <QDialog>
#include "storage/JsonStorage.h"

class QSpinBox;
class QTextEdit;
class QComboBox;
class QLineEdit;

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    SettingsDialog(
        const AppConfig& config,
        const QStringList& blacklist,
        const QStringList& whitelist,
        QWidget* parent = nullptr);

    AppConfig appConfig() const;
    QStringList blacklist() const;
    QStringList whitelist() const;

private:
    static QStringList linesFromText(const QString& text);

    QSpinBox* m_focusMinutes = nullptr;
    QSpinBox* m_breakMinutes = nullptr;
    QSpinBox* m_monitorInterval = nullptr;
    QComboBox* m_aiProviderBox = nullptr;
    QLineEdit* m_openAiBaseUrl = nullptr;
    QLineEdit* m_openAiModel = nullptr;
    QLineEdit* m_openAiApiKeyEnv = nullptr;
    QTextEdit* m_blacklistEdit = nullptr;
    QTextEdit* m_whitelistEdit = nullptr;
};
