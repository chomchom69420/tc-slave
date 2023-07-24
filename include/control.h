/*
* This file is used to set the control mode for the ESP32 
*/
#include <ArduinoJson.h>

enum ControlMode
{
    AUTO,               //Auto is when the slave is not receiving commands, and running on its own
    DICTATED            //Dictated is when slave is receiving commands, either from Master or manually through the Local Control Panel (LCP)
};

void setControlMode(JsonObject &parsed);

int getControlMode();