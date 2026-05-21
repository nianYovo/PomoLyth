#pragma once

#include <QString>
#include <QStringList>

struct AppConfig {
    int defaultFocusMinutes = 25;
    int defaultBreakMinutes = 5;
    int focusMonitorIntervalSeconds = 3;
    QString aiProvider = "mock";
    QString openAiBaseUrl = "https://api.openai.com/v1";
    QString openAiModel = "gpt-4.1-mini";
    QString openAiApiKeyEnv = "OPENAI_API_KEY";
};

class JsonStorage {
public:
    explicit JsonStorage(QString configDir = "config");

    AppConfig loadAppConfig() const;
    QStringList loadBlacklist() const;
    QStringList loadWhitelist() const;
    bool saveAppConfig(const AppConfig& config) const;
    bool saveBlacklist(const QStringList& keywords) const;
    bool saveWhitelist(const QStringList& keywords) const;

private:
    QString readTextFile(const QString& fileName) const;
    bool writeTextFile(const QString& fileName, const QString& content) const;
    QStringList loadStringArray(const QString& fileName, const QStringList& fallback) const;
    bool saveStringArray(const QString& fileName, const QStringList& values) const;

    QString m_configDir;
};
