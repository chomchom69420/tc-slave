/*
* This file is used to set the control mode for the ESP32 
*/
#include <ArduinoJson.h>

enum ControlMode
{
    AUTO,               //Auto is when the slave is not receiving commands, and running on its own
    DICTATED            //Dictated is when slave is receiving commands, either from Master or manually through the Local Control Panel (LCP)
};

/*
JSON format:
{
    "panel_id":1234,
    "mode": "auto"
}
}
*/
void setControlMode(JsonObject &parsed);

int getControlMode();

/*
Manually sets a control mode
This overloaded setControlMode() function will be used to toggle control mode with master coming Online / going Offline
*/
void setControlMode(ControlMode mode);