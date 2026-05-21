#pragma once

#include "ai/IAiClient.h"

class MockAiClient : public IAiClient {
public:
    QString chat(const QString& prompt) override;
};
