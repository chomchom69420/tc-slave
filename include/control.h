/*
* This file is used to set the control mode for the ESP32 
*/
#include <ArduinoJson.h>

enum ControlMode
{
    AUTO, MANUAL
};

void setControlMode(JsonObject &parsed);

int getControlMode();