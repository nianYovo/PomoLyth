#include "core/EventBus.h"

EventBus::EventBus(QObject* parent) : QObject(parent) {
    qRegisterMetaType<AppEvent>("AppEvent");
    qRegisterMetaType<AppEventType>("AppEventType");
}

void EventBus::publish(const AppEvent& event) {
    emit eventPublished(event);
}
