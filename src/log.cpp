#include "log.h"

void log(const char* topic, const char* payload) {
    mqtt_publish(topic, payload);
}