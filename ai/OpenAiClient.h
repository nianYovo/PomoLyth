#pragma once

#include "ai/IAiClient.h"
#include "storage/JsonStorage.h"

class OpenAiClient : public IAiClient {
public:
    explicit OpenAiClient(AppConfig config);

    QString chat(const QString& prompt) override;

private:
    QString endpoint() const;
    QString apiKey() const;

    AppConfig m_config;
};
