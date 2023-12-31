#include "control.h"
#include "control_mqtt.h"

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
        const char* message="Invalid panel ID";
        const char* topic = "/status/logs";
        log(topic, message);
        return;
    }
    
    //Else
    if(parsed["mode"]=="auto")
    {
        controlMode = ControlMode::AUTO;
    }
    else if(parsed["mode"]=="manual")
    {
        controlMode = ControlMode::DICTATED;
    }
    else
    {
        //Invalid control mode, log to MQTT aand return
        char msg[] = "Invalid control mode";
        log("/status/logs", msg);
    }
}

void setControlModeEnum(ControlMode mode)
{
    controlMode = mode;
}