//This is the header file to the .cpp file containing the internal FSM of the LED
#include "control.h"
#include "delay.h"

/*
initSlave() function is used to initialize the slave struct 
It sets the slave id, opposite slave id, panel id
Initializes all timers to 0
Initializers timer numbers
- primary, overhead: timer 0
- secondary: timer 1
- spare: timer 2
Sets the lamp pins according to defs in configurations.h

This function should be exectued in void setup()
*/
void initSlave();

/*
Initializes mode to MODE_MULTIDIRECTION and number of slaves to 1
This function should be exectued in void setup()
*/
void initEnvironment();

/*
Initializes all lamps by putting them in OFF state
*/
void initLamp();

/*
Updates the FSM for both the primary lamp and the secondary lamp
*/
void signals_fsm_update();

/*
Executes the commanded state
Moves the current state into the commanded state
*/
void moveToState();

/*
Returns the state of the specific lamp in the lamp group
*/
bool signals_get_lamp_status(LampID id, Lamp_Channel channel);


/*
Return the mode from the environment struct instance 
*/
int getMode();

/*
Returns the total number of slaves connected
*/
int getTotalSlaves();

/*
Sets the total number of slaves
*/
void setTotalSlaves(int n);
/*
Adds a slave to the current number of slaves
Usually used when a slave is added to the network
*/
void addSlave();

/*
Drops a slave from the current total number of slaves
Usually used when a slave drops from the network
*/
void dropSlave();


                                    /* LAMP CONFIG SECTION  */

/* 
This section is used for configuring the lamps in the slave. True means the lamp is enabled, false means the lamp is disabled
If all the lamps are disabled, then the set of lamps (primary/secondary/overhead/spare) is not connected
This is just to store the intended configuration of the lamps in the slave
The actual checking of whether a lamp is connected / not connected is performed in the lamp_checker
*/

typedef struct lamp_config
{
    // Lamp enables
    bool red;
    bool amber;
    bool green_fwd, green_l, green_r;
};

/*
JSON format for lamp configuration:
{
    "1": {
        "primary": {
            "red":1,
            "amber":1, 
            "green": {
                "fwd":1,
                "l":0,
                "r":0,
            }    
        },
        "secondary": {
            "red":1,
            "amber":1, 
            "green": {
                "fwd":1,
                "l":0,
                "r":0,
            }
        },
        "overhead": {
            "red":1,
            "amber":1, 
            "green": {
                "fwd":1,
                "l":0,
                "r":0,
            }
        },
        "spare": {
            "red":1,
            "amber":1, 
            "green": {
                "fwd":1,
                "l":0,
                "r":0,
            }
        }
    }

    "2": {
        ...
    }
}
*/

bool get_lamp_enable(LampID id, Lamp_Channel channel);

// void signals_config_lamps(JsonObject& parsed);

lamp_config signals_get_lamp_config(LampID id);

/*
Used to initialize the lamp_config
*/
void signals_init_lamp_config();