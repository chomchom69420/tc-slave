#include <Arduino.h>
#include <PubSubClient.h>
// #include "mqtt.h"
// #include "delay.h"
// #include "signals.h"
// #include "configurations.h"
// #include "control.h"
// #include "lamp_checker.h"
#include "timer.h"

long now, last_time;

void setup()
{
    Serial.begin(9600);

    //Setup mqtt
    mqtt_setup();
    mqtt_reconnect();

    //Initialize
    signals_init_slave();
    signals_init_environment();
    signals_init_lamp();
    delay_init();
    timer_hardware_init();
    lamp_checker_init();

    last_time = millis();
}

void loop()
{
    now = millis();
    if (!mqtt_pubsubloop())
        mqtt_reconnect();

    
    if (now - last_time > 30000)
    {
        mqtt_publish_state();
        lamp_checker_log_health();
        lamp_checker_log_lamp_status();
        last_time = now;
    }

    // FSM is used only when master is offline in AUTO mode. Otherwise, always use direct commands
    if (getControlMode() == ControlMode::AUTO && !mqtt_master_online())
    {
        Serial.println("In manual mode (master is offline or LCP is active)");
        signals_fsm_update();
    }

}