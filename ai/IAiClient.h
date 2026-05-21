#pragma once

#include <QString>

class IAiClient {
public:
    virtual ~IAiClient() = default;
    virtual QString chat(const QString& prompt) = 0;
};
