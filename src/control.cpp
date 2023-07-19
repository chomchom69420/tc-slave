#include "control.h"
#include "configurations.h"
#include "mqtt.h"

//Set Control mode to Auto by default
static volatile int controlMode = ControlMode::AUTO; 

int getControlMode()
{
    return controlMode;
}

void setControlMode(JsonObject &parsed)
{
    if(parsed["panel_id"]!=PANEL_ID)
    {
        //Invalid panel, ignore
        String msg="Invalid panel ID";
        mqtt_log(msg);
        return;
    }
    
    //Else
    if(parsed["mode"]=="auto")
    {
        controlMode = ControlMode::AUTO;
    }
    else if(parsed["mode"]=="manual")
    {
        controlMode = ControlMode::MANUAL;
    }
    else
    {
        //Invalid control mode, log to MQTT aand return
        String msg = "Invalid control mode";
        mqtt_log(msg);
    }
}