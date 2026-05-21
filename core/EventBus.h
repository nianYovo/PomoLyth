#pragma once

#include <QObject>
#include "models/AppEvent.h"

class EventBus : public QObject {
    Q_OBJECT

public:
    explicit EventBus(QObject* parent = nullptr);
    void publish(const AppEvent& event);

signals:
    void eventPublished(const AppEvent& event);
};
