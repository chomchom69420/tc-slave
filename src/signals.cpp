#include "signals.h"
#include "delay.h"
#include "Arduino.h"
#include "mqtt.h"
#include "timer.h"
#include "configurations.h"

typedef struct lamp
{
    // Timer Configuration info
    int timer_red;
    int timer_green;
    int timer_yellow;
    int timer_Num; // Stores which timer is being assigned

    // GPIO configuration info
    int redPin;
    int greenFwdPin;
    int greenLeftPin;
    int greenRightPin;
    int yellowPin;

    // Current state info
    int current_state;
    int commanded_state;
    int time_elapsed;
    int time_left;
};

struct slaveStruct
{
    // Slave general configuration info
    int slaveId;
    int oppSlaveId; // ID of opposite slave (-1 if none)
    int panelId;

    lamp primary;
    lamp secondary;
    lamp overhead;
    lamp spare;
} Slave;

// Stores the environment data
struct environmentStruct
{

    int n_slaves;
    int mode;

} Traffic;

static void setOppositeSlaveID();

void initEnvironment()
{
    // Set n_slaves to at least 1 (itself)
    Traffic.n_slaves = 1;
    Traffic.mode = MODE_MULTIDIRECTION;
}

void setEnvironment(JsonObject &parsed)
{
    // This function will be called when the slave receives a signal command from the master
    Traffic.n_slaves = parsed["n"];
    Traffic.mode = parsed["mode"];
}

void initSlave()
{
    // This function is used to initialize the struct Slave
    Slave.slaveId = SLAVE_ID;
    setOppositeSlaveID();
    Slave.panelId = PANEL_ID;

    /* PRIMARY */
    // Initialize timers to 0
    Slave.primary.timer_green = 0;
    Slave.primary.timer_red = 0;
    Slave.primary.timer_yellow = 0;

    // Initialize timer number
    Slave.primary.timer_Num = 0;

    // Intiailize GPIO configuration to configurations file info
    Slave.primary.greenFwdPin = PRIMARY_GREEN_FWD;
    Slave.primary.greenLeftPin = PRIMARY_GREEN_LEFT;
    Slave.primary.greenRightPin = PRIMARY_GREEN_RIGHT;
    Slave.primary.redPin = PRIMARY_RED;
    Slave.primary.yellowPin = PRIMARY_AMBER;

    /* SECONDARY */
    // Initialize timers to 0
    Slave.secondary.timer_green = 0;
    Slave.secondary.timer_red = 0;
    Slave.secondary.timer_yellow = 0;

    // Initialize timer number
    Slave.secondary.timer_Num = 1;

    // Intiailize GPIO configuration to configurations file info
    Slave.secondary.greenFwdPin = SECONDARY_GREEN_FWD;
    Slave.secondary.greenLeftPin = SECONDARY_GREEN_LEFT;
    Slave.secondary.greenRightPin = SECONDARY_GREEN_RIGHT;
    Slave.secondary.redPin = SECONDARY_RED;
    Slave.secondary.yellowPin = SECONDARY_AMBER;

    /* Overhead -- this is because overhead is basically the as primary */
    Slave.overhead = Slave.primary;

    /* spare */
    // Initialize timers to 0
    Slave.spare.timer_green = 0;
    Slave.spare.timer_red = 0;
    Slave.spare.timer_yellow = 0;

    // Initialize timer number
    Slave.spare.timer_Num = 2;

    // Intiailize GPIO configuration to configurations file info
    Slave.spare.greenFwdPin = SPARE_GREEN_FWD;
    Slave.spare.greenLeftPin = SPARE_GREEN_LEFT;
    Slave.spare.greenRightPin = SPARE_GREEN_RIGHT;
    Slave.spare.redPin = SPARE_RED;
    Slave.spare.yellowPin = SPARE_AMBER;
}

static void setOppositeSlaveID()
{
    // if n = even, opp = self + n/2
    if (Traffic.n_slaves % 2 == 0)
        Slave.oppSlaveId = Slave.slaveId + Traffic.n_slaves / 2;

    // if n=odd, opp = self + floor(n/2) unless self = 1 ==> then single no opp
    else
    {
        if (Slave.slaveId == 1)
            Slave.oppSlaveId = -1; // no opposite slave
        else
            Slave.oppSlaveId = Slave.slaveId + (int)Traffic.n_slaves / 2;
    }
}

void setSlave(JsonObject &parsed)
{
    setOppositeSlaveID();
    char s[10];
    sprintf(s, "S%d", Slave.slaveId);
    Slave.primary.commanded_state = parsed[s];
    sprintf(s, "S%d", Slave.oppSlaveId);
    Slave.secondary.commanded_state = parsed[s];

    // Store the timer infos
    sprintf(s, "t%d", Slave.slaveId);
    JsonObject &primary_timer = parsed[s];
    Slave.primary.timer_red = primary_timer["red"];
    Slave.primary.timer_green = primary_timer["green"];
    // Slave.primary.timer_yellow = timer["yellow"];

    sprintf(s, "t%d", Slave.oppSlaveId);
    JsonObject &secondary_timer = parsed[s];
    Slave.secondary.timer_red = secondary_timer["red"];
    Slave.secondary.timer_green = secondary_timer["green"];
}

void initLamp()
{
    // Start every signal in OFF state
    Slave.primary.commanded_state = LampState::OFF;
    Slave.secondary.commanded_state = LampState::OFF;
    Slave.spare.commanded_state = LampState::OFF;
}

void executeCommandedState_modeDependent()
{
    /*  PRIMARY  */
    // if the commanded state is same as the current state, don't do anything
    if (Slave.secondary.current_state != Slave.secondary.commanded_state)
    {
        // go to commanded state
        Slave.primary.current_state = Slave.primary.commanded_state;
        if (Slave.primary.current_state == LampState::RED)
        {
            // manually turn on the red lamp
            digitalWrite(Slave.primary.redPin, 1);

            //..and manually turn off the green lamps
            digitalWrite(Slave.primary.greenFwdPin, 0);
            digitalWrite(Slave.primary.greenLeftPin, 0);
            digitalWrite(Slave.primary.greenRightPin, 0);
        }
        else if (Slave.primary.current_state == LampState::GREEN)
        {
            // Manually turn ON/OFF the lights
            digitalWrite(Slave.primary.redPin, 0);

            if (Traffic.mode == MODE_MULTIDIRECTION)
            {
                digitalWrite(Slave.primary.greenFwdPin, 1);
                digitalWrite(Slave.primary.greenLeftPin, 1);
                digitalWrite(Slave.primary.greenRightPin, 1);
            }
            else if (Traffic.mode == MODE_STRAIGHT_ONLY)
            {
                digitalWrite(Slave.primary.greenFwdPin, 1);
                digitalWrite(Slave.primary.greenLeftPin, 0);
                digitalWrite(Slave.primary.greenRightPin, 0);
            }
        }
    }

    /*  secondary  */
    // if the commanded state is same as the current state, don't do anything
    if (Slave.secondary.current_state != Slave.secondary.commanded_state)
    {
        // go to commanded state
        Slave.secondary.current_state = Slave.secondary.commanded_state;
        if (Slave.secondary.current_state == LampState::RED)
        {
            // manually turn on the red lamp
            digitalWrite(Slave.secondary.redPin, 1);

            //..and manually turn off the green lamps
            digitalWrite(Slave.secondary.greenFwdPin, 0);
            digitalWrite(Slave.secondary.greenLeftPin, 0);
            digitalWrite(Slave.secondary.greenRightPin, 0);
        }
        else if (Slave.secondary.current_state == LampState::GREEN)
        {
            // Manually turn ON/OFF the lights
            digitalWrite(Slave.secondary.redPin, 0);

            if (Traffic.mode == MODE_MULTIDIRECTION)
            {
                digitalWrite(Slave.secondary.greenFwdPin, 1);
                digitalWrite(Slave.secondary.greenLeftPin, 1);
                digitalWrite(Slave.secondary.greenRightPin, 1);
            }
            else if (Traffic.mode == MODE_STRAIGHT_ONLY)
            {
                digitalWrite(Slave.secondary.greenFwdPin, 1);
                digitalWrite(Slave.secondary.greenLeftPin, 0);
                digitalWrite(Slave.secondary.greenRightPin, 0);
            }
        }
    }
}

// TODO: Add amber states to the state machine
void primary_lamp_fsm_update()
{
    switch (Slave.primary.current_state)
    {
    case LampState::OFF:
        // Turn everything off
        digitalWrite(Slave.primary.redPin, 0);
        digitalWrite(Slave.primary.redPin, 1);
        digitalWrite(Slave.primary.greenFwdPin, 0);
        digitalWrite(Slave.primary.greenLeftPin, 0);
        digitalWrite(Slave.primary.greenRightPin, 0);
        break;

    case LampState::RED:
        // Turn ON red LED
        digitalWrite(Slave.primary.redPin, 1);

        // Turn OFF all greens
        digitalWrite(Slave.primary.greenFwdPin, 0);
        digitalWrite(Slave.primary.greenLeftPin, 0);
        digitalWrite(Slave.primary.greenRightPin, 0);
        if (delay_is_done(Slave.primary.timer_Num))
        {
            Slave.primary.current_state = LampState::GREEN;
            delay_set(Slave.primary.timer_Num, Slave.primary.timer_green);
            return;
        }
        break;

    case LampState::GREEN:
        // Turn ON greens
        digitalWrite(Slave.primary.greenFwdPin, 1);
        if (Traffic.mode == MODE_MULTIDIRECTION)
        {
            digitalWrite(Slave.primary.greenLeftPin, 1);
            digitalWrite(Slave.primary.greenRightPin, 1);
        }

        // Turn OFF red
        digitalWrite(Slave.primary.redPin, 0);

        if (delay_is_done(Slave.primary.timer_Num))
        {
            Slave.primary.current_state = LampState::RED;
            delay_set(Slave.primary.timer_Num, Slave.primary.timer_red);
            return;
        }
        break;

    default:
        break;
    }
}

void secondary_lamp_fsm_update()
{
    // if oppSlaveID is -1 then just turn OFF all lamps
    if (Slave.oppSlaveId == -1)
    {
        digitalWrite(Slave.secondary.redPin, 0);
        digitalWrite(Slave.secondary.redPin, 1);
        digitalWrite(Slave.secondary.greenFwdPin, 0);
        digitalWrite(Slave.secondary.greenLeftPin, 0);
        digitalWrite(Slave.secondary.greenRightPin, 0);
    }
    else
    {
        switch (Slave.secondary.current_state)
        {
        case LampState::OFF:
            // Turn everything off
            digitalWrite(Slave.secondary.redPin, 0);
            digitalWrite(Slave.secondary.redPin, 1);
            digitalWrite(Slave.secondary.greenFwdPin, 0);
            digitalWrite(Slave.secondary.greenLeftPin, 0);
            digitalWrite(Slave.secondary.greenRightPin, 0);
            break;

        case LampState::RED:
            // Turn ON red LED
            digitalWrite(Slave.secondary.redPin, 1);

            // Turn OFF all greens
            digitalWrite(Slave.secondary.greenFwdPin, 0);
            digitalWrite(Slave.secondary.greenLeftPin, 0);
            digitalWrite(Slave.secondary.greenRightPin, 0);
            if (delay_is_done(Slave.secondary.timer_Num))
            {
                Slave.secondary.current_state = LampState::GREEN;
                delay_set(Slave.secondary.timer_Num, Slave.secondary.timer_green);
                return;
            }
            break;

        case LampState::GREEN:
            // Turn ON greens
            digitalWrite(Slave.secondary.greenFwdPin, 1);
            if (Traffic.mode == MODE_MULTIDIRECTION)
            {
                digitalWrite(Slave.secondary.greenLeftPin, 1);
                digitalWrite(Slave.secondary.greenRightPin, 1);
            }

            // Turn OFF red
            digitalWrite(Slave.secondary.redPin, 0);

            if (delay_is_done(Slave.secondary.timer_Num))
            {
                Slave.secondary.current_state = LampState::RED;
                delay_set(Slave.secondary.timer_Num, Slave.secondary.timer_red);
                return;
            }
            break;

        default:
            break;
        }
    }
}

int getPrimaryState()
{
    return Slave.primary.current_state;
}

int getSecondaryState()
{
    return Slave.secondary.current_state;
}

int getOverheadState()
{
    return Slave.primary.current_state;
}

int getSpareState()
{
    return Slave.spare.current_state;
}

int getTimerNum(int lampID)
{
    if (lampID == LampID::PRIMARY || lampID == LampID::OVERHEAD)
        return Slave.primary.timer_Num;
    else if (lampID == LampID::SECONDARY)
        return Slave.secondary.timer_Num;
    else if (lampID == LampID::SPARE)
        return Slave.spare.timer_Num;
    else
        return -1;
}

unsigned int getElapsedTime(int lampID)
{
    if (lampID == LampID::PRIMARY || lampID == LampID::OVERHEAD)
        return delay_get(Slave.primary.timer_Num);
    else if (lampID == LampID::SECONDARY)
        return delay_get(Slave.secondary.timer_Num);
    else if (lampID == LampID::SPARE)
        return delay_get(Slave.spare.timer_Num);
}

int getTimerValues(int lampID, int colourID)
{
    if (lampID == LampID::PRIMARY || lampID == LampID::OVERHEAD)
    {
        if (colourID == LampState::RED)
            return Slave.primary.timer_red;
        else if (colourID == LampState::AMBER)
            return Slave.primary.timer_yellow;
        else if (colourID == LampState::GREEN)
            return Slave.primary.timer_green;
    }

    else if (lampID == LampID::SECONDARY)
    {
        if (colourID == LampState::RED)
            return Slave.secondary.timer_red;
        else if (colourID == LampState::AMBER)
            return Slave.secondary.timer_yellow;
        else if (colourID == LampState::GREEN)
            return Slave.secondary.timer_green;
    }

    else if (lampID == LampID::SPARE)
    {
        if (colourID == LampState::RED)
            return Slave.spare.timer_red;
        else if (colourID == LampState::AMBER)
            return Slave.spare.timer_yellow;
        else if (colourID == LampState::GREEN)
            return Slave.spare.timer_green;
    }

    return -1; // if doesn't return anywhere
}

unsigned int getRemainingTime(int lampID)
{
    if (lampID == LampID::PRIMARY || lampID == LampID::OVERHEAD)
    {
        if (Slave.primary.current_state == LampState::RED)
            return Slave.primary.timer_red - delay_get(Slave.primary.timer_Num);
        else if (Slave.primary.current_state == LampState::AMBER)
            return Slave.primary.timer_yellow - delay_get(Slave.primary.timer_Num);
        else if (Slave.primary.current_state == LampState::GREEN)
            return Slave.primary.timer_green - delay_get(Slave.primary.timer_Num);
    }

    else if (lampID == LampID::SECONDARY)
    {
        if (Slave.secondary.current_state == LampState::RED)
            return Slave.secondary.timer_red - delay_get(Slave.secondary.timer_Num);
        else if (Slave.secondary.current_state == LampState::AMBER)
            return Slave.secondary.timer_yellow - delay_get(Slave.secondary.timer_Num);
        else if (Slave.secondary.current_state == LampState::GREEN)
            return Slave.secondary.timer_green - delay_get(Slave.secondary.timer_Num);
    }

    else if (lampID == LampID::SPARE)
    {
        if (Slave.spare.current_state == LampState::RED)
            return Slave.spare.timer_red - delay_get(Slave.spare.timer_Num);
        else if (Slave.spare.current_state == LampState::AMBER)
            return Slave.spare.timer_yellow - delay_get(Slave.spare.timer_Num);
        else if (Slave.spare.current_state == LampState::GREEN)
            return Slave.spare.timer_green - delay_get(Slave.spare.timer_Num);
    }
}

int getMode(){
    return Traffic.mode;
}

int getTotalSlaves(){
    return Traffic.n_slaves;
}

void addSlave(){
    Traffic.n_slaves++;
}

void dropSlave(){
    Traffic.n_slaves--;
}

void setTotalSlaves(int n){
    Traffic.n_slaves = n;
}