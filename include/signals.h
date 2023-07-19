//This is the header file to the .cpp file containing the internal FSM of the LED
#include "ArduinoJson.h"

enum LampID
{
    PRIMARY = 0,
    SECONDARY = 1,
    OVERHEAD = 2,
    SPARE = 3
};

enum LampState
{
    RED,
    AMBER,
    GREEN,
    OFF
};

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
void lampInit();

/*
Takes a JsonObject and sets the environment
Reads mode and n_slaves from the JsonObject
This needs to be called before setSlave()
*/
void setEnvironment(ArduinoJson::JsonObject &parsed);

/*
Takes a JsonObject and sets the slave instance
Sets the opposite slave ID
Sets the commanded states for primary and secondary
Sets the timer values for primary and secondary
*/
void setSlave(ArduinoJson::JsonObject &parsed);

/*
Updates the FSM for the primary lamp
*/
void primary_lamp_fsm_update();

/*
Updates the FSM for the secondary lamp
*/
void secondary_lamp_fsm_update();

/*
Executes the commanded state
Moves the current state into the commanded state
*/
void executeCommandedState_modeDependent();

/*
Returns the state of the primary lamp
*/
int getPrimaryState();

/*
Returns the state of the secondary lamp
*/
int getSecondaryState();

/*
Returns the state of the overhead lamp
*/
int getOverheadState();

/*
Returns the state of the spare lamp
*/
int getSpareState();

/*
Returns the timer number of the specific lamp with lampID in LampID enum
*/
int getTimerNum(int lampID);

/*
Returns the elapsed time for the specific lamp
*/
unsigned int getElapsedTime(int lampID);

/*
Returns the red, green, amber timer values
lampID in LampID
colour in LampState
*/
int getTimerValues(int lampID, int colourID);

/*
Returns the remaining time for the specific lamp
*/
unsigned int getRemainingTime(int lampID);

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