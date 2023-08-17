/*
* This file is used to set the control mode for the ESP32 
*/
#include "log.h"

/*
JSON format:
{
    "panel_id":1234,
    "mode": "auto"
}
*/
// void setControlMode(JsonObject &parsed);
int getControlMode();

/*
Manually sets a control mode
This overloaded setControlMode() function will be used to toggle control mode with master coming Online / going Offline
*/
void setControlModeEnum(ControlMode mode);

void setControlMode(JsonObject &parsed);