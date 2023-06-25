#include <Arduino.h>
#include <PubSubClient.h>
#include "mqtt.h"
#include "delay.h"
#include "timer.h"
#include "signals.h"
#include "configurations.h"

long now, last_time;

void setup()
{
    Serial.begin(9600);
    pinMode(13, OUTPUT);
    mqtt_setup();
    reconnect();
    digitalWrite(13, LOW);
    last_time = millis();

    //Setup the timers
    timer_update(DEFAULT_TIMER_RED, DEFAULT_TIMER_GREEN);
}

void loop(){
    now = millis();
    if(!pubsubloop())  reconnect(); 
    if(now - last_time > 30000)
    {
        // reconnect();
        publish_state();
        last_time = now;
    }

    //Use fsm only if master is offline
    if(!mqtt_master_online())
    {
        signals_update();
    }
}