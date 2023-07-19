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

void primary_lamp_fsm_update();

void secondary_lamp_fsm_update();

void lampInit();

void initSlave();

void initEnvironment();

void setEnvironment(ArduinoJson::JsonObject &parsed);

void setSlave(ArduinoJson::JsonObject &parsed);

void executeCommandedState_modeDependent();

int getPrimaryState();

int getSecondaryState();

int getOverheadState();

int getSpareState();

int getTimerNum(int lampID);

unsigned int getElapsedTime(int lampID);

int getTimerValues(int lampID, int colourID);