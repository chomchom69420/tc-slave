#if 0

#include "configurations.h"
#include "ArduinoJson.h"

class Environment 
{
public:
    int n_slaves;     //number of slaves in the environment
    
}

class Lamp
{
private:
    int time_elapsed;
    int time_left;

    // GPIO configuration info
    int redPin;
    int greenFwdPin;
    int greenLeftPin;
    int greenRightPin;
    int amberPin;

public:
    const char *name; //"primary", "secondary", "overhead", "spare"
    int timer_red;
    int timer_green;
    int timer_amber;
    int timer_Num; // Stores which timer is being assigned in the delay api

    // Current state info
    int current_state;

public:
    Lamp(const char *name)
    {
        this->name = name;
        timer_red = 0;
        timer_green = 0;
        timer_amber = 0;
        timer_Num = -1; // do not assign any timer

        if (name == "primary")
        {
            redPin = PRIMARY_RED;
            amberPin = PRIMARY_AMBER;
            greenFwdPin = PRIMARY_GREEN_FWD;
            greenLeftPin = PRIMARY_GREEN_LEFT;
            greenRightPin = PRIMARY_GREEN_RIGHT;
        }

        else if (name == "secondary")
        {
            redPin = SECONDARY_RED;
            amberPin = SECONDARY_AMBER;
            greenFwdPin = SECONDARY_GREEN_FWD;
            greenLeftPin = SECONDARY_GREEN_LEFT;
            greenRightPin = SECONDARY_GREEN_RIGHT;
        }

        else if (name == "spare")
        {
            redPin = SPARE_RED;
            amberPin = SPARE_AMBER;
            greenFwdPin = SPARE_GREEN_FWD;
            greenLeftPin = SPARE_GREEN_LEFT;
            greenRightPin = SPARE_GREEN_RIGHT;
        }

        else if (name == "overhead")
        {
            redPin = OVERHEAD_RED;
            amberPin = OVERHEAD_AMBER;
            greenFwdPin = OVERHEAD_GREEN_FWD;
            greenLeftPin = OVERHEAD_GREEN_LEFT;
            greenRightPin = OVERHEAD_GREEN_RIGHT;
        }

        current_state = SLAVE_STATE_IDLE;
    };
};

class Slave
{
public:
    const char *name; // can store the name of the road, crossing, etc
    Lamp primary = Lamp("primary");
    Lamp secondary = Lamp("secondary");
    Lamp overhead = Lamp("overhead");
    Lamp spare = Lamp("spare");

    int slaveId;
    int oppslaveId;

public:
    Slave(const char *name)
    {
        this->name = name;
        slaveId = SLAVE_ID;
        oppslaveId = -1; // init with no opp slave
    }
};

#endif