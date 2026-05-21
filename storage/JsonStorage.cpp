#include "storage/JsonStorage.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <utility>

JsonStorage::JsonStorage(QString configDir) : m_configDir(std::move(configDir)) {}

AppConfig JsonStorage::loadAppConfig() const {
    AppConfig config;
    const QJsonDocument doc = QJsonDocument::fromJson(readTextFile("app_config.json").toUtf8());
    if (!doc.isObject()) {
        return config;
    }

    const QJsonObject obj = doc.object();
    config.defaultFocusMinutes = obj.value("defaultFocusMinutes").toInt(config.defaultFocusMinutes);
    config.defaultBreakMinutes = obj.value("defaultBreakMinutes").toInt(config.defaultBreakMinutes);
    config.focusMonitorIntervalSeconds = obj.value("focusMonitorIntervalSeconds").toInt(config.focusMonitorIntervalSeconds);
    config.aiProvider = obj.value("aiProvider").toString(config.aiProvider);
    config.openAiBaseUrl = obj.value("openAiBaseUrl").toString(config.openAiBaseUrl);
    config.openAiModel = obj.value("openAiModel").toString(config.openAiModel);
    config.openAiApiKeyEnv = obj.value("openAiApiKeyEnv").toString(config.openAiApiKeyEnv);
    return config;
}

QStringList JsonStorage::loadBlacklist() const {
    return loadStringArray("blacklist.json", {"bilibili", "youtube", "steam", "tiktok", "douyin", "game", "video"});
}

QStringList JsonStorage::loadWhitelist() const {
    return loadStringArray("whitelist.json", {});
}

bool JsonStorage::saveAppConfig(const AppConfig& config) const {
    QJsonObject obj;
    obj.insert("defaultFocusMinutes", config.defaultFocusMinutes);
    obj.insert("defaultBreakMinutes", config.defaultBreakMinutes);
    obj.insert("focusMonitorIntervalSeconds", config.focusMonitorIntervalSeconds);
    obj.insert("aiProvider", config.aiProvider);
    obj.insert("openAiBaseUrl", config.openAiBaseUrl);
    obj.insert("openAiModel", config.openAiModel);
    obj.insert("openAiApiKeyEnv", config.openAiApiKeyEnv);
    return writeTextFile("app_config.json", QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Indented)));
}

bool JsonStorage::saveBlacklist(const QStringList& keywords) const {
    return saveStringArray("blacklist.json", keywords);
}

bool JsonStorage::saveWhitelist(const QStringList& keywords) const {
    return saveStringArray("whitelist.json", keywords);
}

QString JsonStorage::readTextFile(const QString& fileName) const {
    QFile file(QDir(m_configDir).filePath(fileName));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

bool JsonStorage::writeTextFile(const QString& fileName, const QString& content) const {
    QDir().mkpath(m_configDir);
    QFile file(QDir(m_configDir).filePath(fileName));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }
    file.write(content.toUtf8());
    return true;
}

QStringList JsonStorage::loadStringArray(const QString& fileName, const QStringList& fallback) const {
    QStringList items;
    const QJsonDocument doc = QJsonDocument::fromJson(readTextFile(fileName).toUtf8());
    if (!doc.isArray()) {
        return fallback;
    }

    for (const QJsonValue& value : doc.array()) {
        const QString item = value.toString().trimmed();
        if (!item.isEmpty()) {
            items.append(item);
        }
    }
    return items;
}

bool JsonStorage::saveStringArray(const QString& fileName, const QStringList& values) const {
    QJsonArray array;
    for (const QString& value : values) {
        const QString trimmed = value.trimmed();
        if (!trimmed.isEmpty()) {
            array.append(trimmed);
        }
    }
    return writeTextFile(fileName, QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Indented)));
}
