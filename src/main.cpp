#include <Arduino.h>
#include <PubSubClient.h>
#include "mqtt.h"
#include "delay.h"
#include "signals.h"
#include "configurations.h"
#include "control.h"

long now, last_time;

void setup()
{
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    mqtt_setup();
    reconnect();
    digitalWrite(13, LOW);
    last_time = millis();

    //Initialize
    initSlave();
    initEnvironment();
    initLamp();
    delay_init();
}

void loop()
{
    now = millis();
    if (!pubsubloop())
        reconnect();
    if (now - last_time > 30000)
    {
        publish_state();
        last_time = now;
    }

    // FSM is used only when master is offline in AUTO mode. Otherwise, always use direct commands
    if (getControlMode() == ControlMode::AUTO && !mqtt_master_online())
    {
        primary_lamp_fsm_update();
        secondary_lamp_fsm_update();
    }
}