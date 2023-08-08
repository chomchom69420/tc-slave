#include "log.h"

void log(char* topic, char* payload) {
    mqtt_publish(topic, payload);
}