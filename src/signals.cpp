#include "signals.h"
#include "delay.h"
#include "Arduino.h"
#include "mqtt.h"
#include "timer.h"
#include "configurations.h"
#include "control.h"

typedef struct lamp
{

public:
    // Timer Configuration info
    int timer_red;
    int timer_green;
    int timer_amber;
    int timer_Num; // Stores which timer is being assigned
    int blink_f;         // Frequency (only for blinker mode)

    // GPIO configuration info
    int redPin;
    int greenFwdPin;
    int greenLeftPin;
    int greenRightPin;
    int amberPin;

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
    Slave.primary.timer_amber = 0;

    // Initialize timer number
    Slave.primary.timer_Num = 0;

    // Intiailize GPIO configuration to configurations file info
    Slave.primary.greenFwdPin = PRIMARY_GREEN_FWD;
    Slave.primary.greenLeftPin = PRIMARY_GREEN_LEFT;
    Slave.primary.greenRightPin = PRIMARY_GREEN_RIGHT;
    Slave.primary.redPin = PRIMARY_RED;
    Slave.primary.amberPin = PRIMARY_AMBER;

    /* SECONDARY */
    // Initialize timers to 0
    Slave.secondary.timer_green = 0;
    Slave.secondary.timer_red = 0;
    Slave.secondary.timer_amber = 0;

    // Initialize timer number
    Slave.secondary.timer_Num = 1;

    // Intiailize GPIO configuration to configurations file info
    Slave.secondary.greenFwdPin = SECONDARY_GREEN_FWD;
    Slave.secondary.greenLeftPin = SECONDARY_GREEN_LEFT;
    Slave.secondary.greenRightPin = SECONDARY_GREEN_RIGHT;
    Slave.secondary.redPin = SECONDARY_RED;
    Slave.secondary.amberPin = SECONDARY_AMBER;

    /* Overhead -- this is because overhead is basically the as primary */
    Slave.overhead = Slave.primary;

    /* spare */
    // Initialize timers to 0
    Slave.spare.timer_green = 0;
    Slave.spare.timer_red = 0;
    Slave.spare.timer_amber = 0;

    // Initialize timer number
    Slave.spare.timer_Num = 2;

    // Intiailize GPIO configuration to configurations file info
    Slave.spare.greenFwdPin = SPARE_GREEN_FWD;
    Slave.spare.greenLeftPin = SPARE_GREEN_LEFT;
    Slave.spare.greenRightPin = SPARE_GREEN_RIGHT;
    Slave.spare.redPin = SPARE_RED;
    Slave.spare.amberPin = SPARE_AMBER;
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
    JsonObject &slaves = parsed["slaves"];
    char s[2];
    sprintf(s, "%d", Slave.slaveId);
    JsonObject &primary = slaves[s];
    sprintf(s, "%d", Slave.oppSlaveId);
    JsonObject &secondary = slaves[s];

    Slave.primary.commanded_state = primary["state"];
    Slave.primary.timer_red = primary["red"];
    Slave.primary.timer_amber = primary["amber"];
    Slave.primary.timer_green = primary["green"];
    if(Slave.primary.commanded_state == SlaveState::BLINKER)
    {
        //parse in the blinker frequency
        Slave.primary.blink_f = primary["blink_f"];
    }

    Slave.secondary.commanded_state = secondary["state"];
    Slave.secondary.timer_red = secondary["red"];
    Slave.secondary.timer_green = secondary["green"];
    Slave.secondary.timer_amber = secondary["amber"];
    if(Slave.secondary.commanded_state == SlaveState::BLINKER)
    {
        //parse in the blinker frequency
        Slave.secondary.blink_f = secondary["blink_f"];
    }

}

void primary_moveToState(bool cmd_state_stored = true, int cmd_state = -1)
{
    if (!cmd_state_stored)
    {
        Slave.primary.commanded_state = cmd_state;
    }

    if (Slave.primary.current_state == Slave.primary.commanded_state)
        return;

    Slave.primary.current_state = Slave.primary.commanded_state;
    switch (Slave.primary.current_state)
    {
    case SlaveState::OFF:
        noTone(Slave.primary.amberPin);
        digitalWrite(Slave.primary.redPin, 0);
        digitalWrite(Slave.primary.amberPin, 0);
        digitalWrite(Slave.primary.greenFwdPin, 0);
        digitalWrite(Slave.primary.greenLeftPin, 0);
        digitalWrite(Slave.primary.greenRightPin, 0);
        break;

    case SlaveState::AUTO_RED:
        noTone(Slave.primary.amberPin);
        digitalWrite(Slave.primary.redPin, 1);
        digitalWrite(Slave.primary.amberPin, 0);
        digitalWrite(Slave.primary.greenFwdPin, 0);
        digitalWrite(Slave.primary.greenLeftPin, 0);
        digitalWrite(Slave.primary.greenRightPin, 0);
        break;

    case SlaveState::AUTO_AMBER:
        noTone(Slave.primary.amberPin);
        digitalWrite(Slave.primary.redPin, 0);
        digitalWrite(Slave.primary.amberPin, 1);
        digitalWrite(Slave.primary.greenFwdPin, 0);
        digitalWrite(Slave.primary.greenLeftPin, 0);
        digitalWrite(Slave.primary.greenRightPin, 0);
        break;

    case SlaveState::AUTO_GREEN:
        digitalWrite(Slave.primary.redPin, 0);
        switch (Traffic.mode)
        {
        case MODE_MULTIDIRECTION:
            digitalWrite(Slave.primary.greenFwdPin, 1);
            digitalWrite(Slave.primary.greenLeftPin, 1);
            digitalWrite(Slave.primary.greenRightPin, 1);
            noTone(Slave.primary.amberPin);
            break;

        case MODE_STRAIGHT_ONLY:
            digitalWrite(Slave.primary.greenFwdPin, 1);
            digitalWrite(Slave.primary.greenLeftPin, 0);
            digitalWrite(Slave.primary.greenRightPin, 0);
            noTone(Slave.primary.amberPin);
            break;
        }
        break;

        /* DICTATED STATES */

    case SlaveState::DICTATED_RED:
        noTone(Slave.primary.amberPin);
        digitalWrite(Slave.primary.redPin, 1);
        digitalWrite(Slave.primary.amberPin, 0);
        digitalWrite(Slave.primary.greenFwdPin, 0);
        digitalWrite(Slave.primary.greenLeftPin, 0);
        digitalWrite(Slave.primary.greenRightPin, 0);
        break;

    case SlaveState::DICTATED_AMBER:
        noTone(Slave.primary.amberPin);
        digitalWrite(Slave.primary.redPin, 0);
        digitalWrite(Slave.primary.amberPin, 1);
        digitalWrite(Slave.primary.greenFwdPin, 0);
        digitalWrite(Slave.primary.greenLeftPin, 0);
        digitalWrite(Slave.primary.greenRightPin, 0);
        break;

    case SlaveState::DICTATED_GREEN:
        digitalWrite(Slave.primary.redPin, 0);
        switch (Traffic.mode)
        {
        case MODE_MULTIDIRECTION:
            digitalWrite(Slave.primary.greenFwdPin, 1);
            digitalWrite(Slave.primary.greenLeftPin, 1);
            digitalWrite(Slave.primary.greenRightPin, 1);
            noTone(Slave.primary.amberPin);
            break;

        case MODE_STRAIGHT_ONLY:
            digitalWrite(Slave.primary.greenFwdPin, 1);
            digitalWrite(Slave.primary.greenLeftPin, 0);
            digitalWrite(Slave.primary.greenRightPin, 0);
            noTone(Slave.primary.amberPin);
            break;
        }
        break;

    case SlaveState::BLINKER:
        tone(Slave.primary.amberPin, Slave.primary.blink_f);
        digitalWrite(Slave.primary.greenFwdPin, 0);
        digitalWrite(Slave.primary.greenLeftPin, 0);
        digitalWrite(Slave.primary.greenRightPin, 0);
        break;

    default:
        break;
    }
}

void secondary_moveToState(bool cmd_state_stored = true, int cmd_state = -1)
{
    if (!cmd_state_stored)
    {
        Slave.secondary.commanded_state = cmd_state;
    }

    if (Slave.secondary.current_state == Slave.secondary.commanded_state)
        return;

    Slave.secondary.current_state = Slave.secondary.commanded_state;
    switch (Slave.secondary.current_state)
    {
    case SlaveState::OFF:
        noTone(Slave.secondary.amberPin);
        digitalWrite(Slave.secondary.redPin, 0);
        digitalWrite(Slave.secondary.amberPin, 0);
        digitalWrite(Slave.secondary.greenFwdPin, 0);
        digitalWrite(Slave.secondary.greenLeftPin, 0);
        digitalWrite(Slave.secondary.greenRightPin, 0);
        break;

    case SlaveState::AUTO_RED:
        noTone(Slave.secondary.amberPin);
        digitalWrite(Slave.secondary.redPin, 1);
        digitalWrite(Slave.secondary.amberPin, 0);
        digitalWrite(Slave.secondary.greenFwdPin, 0);
        digitalWrite(Slave.secondary.greenLeftPin, 0);
        digitalWrite(Slave.secondary.greenRightPin, 0);
        break;

    case SlaveState::AUTO_AMBER:
        noTone(Slave.secondary.amberPin);
        digitalWrite(Slave.secondary.redPin, 0);
        digitalWrite(Slave.secondary.amberPin, 1);
        digitalWrite(Slave.secondary.greenFwdPin, 0);
        digitalWrite(Slave.secondary.greenLeftPin, 0);
        digitalWrite(Slave.secondary.greenRightPin, 0);
        break;

    case SlaveState::AUTO_GREEN:
        digitalWrite(Slave.secondary.redPin, 0);
        switch (Traffic.mode)
        {
        case MODE_MULTIDIRECTION:
            digitalWrite(Slave.secondary.greenFwdPin, 1);
            digitalWrite(Slave.secondary.greenLeftPin, 1);
            digitalWrite(Slave.secondary.greenRightPin, 1);
            noTone(Slave.secondary.amberPin);
            break;

        case MODE_STRAIGHT_ONLY:
            digitalWrite(Slave.secondary.greenFwdPin, 1);
            digitalWrite(Slave.secondary.greenLeftPin, 0);
            digitalWrite(Slave.secondary.greenRightPin, 0);
            noTone(Slave.secondary.amberPin);
            break;
        }
        break;

        /* DICTATED STATES */

    case SlaveState::DICTATED_RED:
        noTone(Slave.secondary.amberPin);
        digitalWrite(Slave.secondary.redPin, 1);
        digitalWrite(Slave.secondary.amberPin, 0);
        digitalWrite(Slave.secondary.greenFwdPin, 0);
        digitalWrite(Slave.secondary.greenLeftPin, 0);
        digitalWrite(Slave.secondary.greenRightPin, 0);
        break;

    case SlaveState::DICTATED_AMBER:
        noTone(Slave.secondary.amberPin);
        digitalWrite(Slave.secondary.redPin, 0);
        digitalWrite(Slave.secondary.amberPin, 1);
        digitalWrite(Slave.secondary.greenFwdPin, 0);
        digitalWrite(Slave.secondary.greenLeftPin, 0);
        digitalWrite(Slave.secondary.greenRightPin, 0);
        break;

    case SlaveState::DICTATED_GREEN:
        digitalWrite(Slave.secondary.redPin, 0);
        switch (Traffic.mode)
        {
        case MODE_MULTIDIRECTION:
            digitalWrite(Slave.secondary.greenFwdPin, 1);
            digitalWrite(Slave.secondary.greenLeftPin, 1);
            digitalWrite(Slave.secondary.greenRightPin, 1);
            noTone(Slave.secondary.amberPin);
            break;

        case MODE_STRAIGHT_ONLY:
            digitalWrite(Slave.secondary.greenFwdPin, 1);
            digitalWrite(Slave.secondary.greenLeftPin, 0);
            digitalWrite(Slave.secondary.greenRightPin, 0);
            noTone(Slave.secondary.amberPin);
            break;
        }
        break;

    case SlaveState::BLINKER:
        tone(Slave.secondary.amberPin, Slave.secondary.blink_f);
        digitalWrite(Slave.secondary.greenFwdPin, 0);
        digitalWrite(Slave.secondary.greenLeftPin, 0);
        digitalWrite(Slave.secondary.greenRightPin, 0);
        break;

    default:
        break;
    }
}

void initLamp()
{
    // Start every signal in OFF state
    Slave.primary.commanded_state = SlaveState::OFF;
    Slave.secondary.commanded_state = SlaveState::OFF;
    Slave.spare.commanded_state = SlaveState::OFF;

    primary_moveToState();
    secondary_moveToState();
}

// TODO: Add amber states to the state machine
static void primary_lamp_fsm_update()
{
    int control = getControlMode();
    switch (Slave.primary.current_state)
    {
    case SlaveState::OFF:
        if (Slave.primary.commanded_state != SlaveState::OFF)
            primary_moveToState();
        break;

    case SlaveState::AUTO_RED:
        if (control == ControlMode::DICTATED)
            primary_moveToState(false, SlaveState::DICTATED_RED);

        if(Slave.primary.commanded_state == SlaveState::BLINKER)
            primary_moveToState();

        if (delay_is_done(Slave.primary.timer_Num))
        {
            Slave.primary.current_state = SlaveState::AUTO_AMBER;
            delay_set(Slave.primary.timer_Num, Slave.primary.timer_amber);
            break;
        }
        break;

    case SlaveState::AUTO_AMBER:
        if (control == ControlMode::DICTATED)
            primary_moveToState(false, SlaveState::DICTATED_AMBER);

        if(Slave.primary.commanded_state == SlaveState::BLINKER)
            primary_moveToState();

        if (delay_is_done(Slave.primary.timer_Num))
        {
            Slave.primary.current_state = SlaveState::AUTO_GREEN;
            delay_set(Slave.primary.timer_Num, Slave.primary.timer_green);
            break;
        }
        break;

    case SlaveState::AUTO_GREEN:
        if(control == ControlMode::DICTATED)
            primary_moveToState(false, SlaveState::DICTATED_GREEN);

        if(Slave.primary.commanded_state == SlaveState::BLINKER)
            primary_moveToState();
        
        if (delay_is_done(Slave.primary.timer_Num))
        {
            Slave.primary.current_state = SlaveState::AUTO_RED;
            delay_set(Slave.primary.timer_Num, Slave.primary.timer_red);
            break;
        }
        break;

    case SlaveState::DICTATED_RED:
        if(control == ControlMode::AUTO)
            primary_moveToState(false, SlaveState::AUTO_RED);

        if (Slave.primary.commanded_state == SlaveState::DICTATED_AMBER)
            primary_moveToState();

        else if(Slave.primary.commanded_state == SlaveState::DICTATED_GREEN)
            primary_moveToState();

        else if(Slave.primary.commanded_state == SlaveState::BLINKER)
            primary_moveToState();

        break;
    
    case SlaveState::DICTATED_AMBER:
        if(control == ControlMode::AUTO)
            primary_moveToState(false, SlaveState::AUTO_AMBER);

        if (Slave.primary.commanded_state == SlaveState::DICTATED_RED)
            primary_moveToState();

        else if(Slave.primary.commanded_state == SlaveState::DICTATED_GREEN)
            primary_moveToState();

        else if(Slave.primary.commanded_state == SlaveState::BLINKER)
            primary_moveToState();

        break;
    
    case SlaveState::DICTATED_GREEN:
        if(control == ControlMode::AUTO)
            primary_moveToState(false, SlaveState::AUTO_GREEN);

        if (Slave.primary.commanded_state == SlaveState::DICTATED_AMBER)
            primary_moveToState();

        else if(Slave.primary.commanded_state == SlaveState::DICTATED_RED)
            primary_moveToState();

        else if(Slave.primary.commanded_state == SlaveState::BLINKER)
            primary_moveToState();

        break;

    case SlaveState::BLINKER:
        //It will loop forever until it has been asked to go to DICTATED_RED (this is basically the red extension part)

        if(Slave.primary.commanded_state == SlaveState::DICTATED_RED)
            primary_moveToState();
        
        break;

    default:
        break;
    }
}

static void secondary_lamp_fsm_update()
{
    switch (Slave.secondary.current_state)
    {
    case SlaveState::OFF:
        if (Slave.secondary.commanded_state != SlaveState::OFF)
            secondary_moveToState();
        break;

    case SlaveState::AUTO_RED:
        if (Slave.secondary.commanded_state != SlaveState::AUTO_RED)
            secondary_moveToState();
        if (delay_is_done(Slave.secondary.timer_Num))
        {
            Slave.secondary.current_state = SlaveState::AUTO_AMBER;
            delay_set(Slave.secondary.timer_Num, Slave.secondary.timer_amber);
            break;
        }
        break;

    case SlaveState::AUTO_AMBER:
        if (Slave.secondary.commanded_state != SlaveState::AUTO_AMBER)
            secondary_moveToState();
        if (delay_is_done(Slave.secondary.timer_Num))
        {
            Slave.secondary.current_state = SlaveState::AUTO_GREEN;
            delay_set(Slave.secondary.timer_Num, Slave.secondary.timer_green);
            break;
        }
        break;

    case SlaveState::AUTO_GREEN:
        if (Slave.secondary.commanded_state != SlaveState::AUTO_GREEN)
            secondary_moveToState();
        if (delay_is_done(Slave.secondary.timer_Num))
        {
            Slave.secondary.current_state = SlaveState::AUTO_RED;
            delay_set(Slave.secondary.timer_Num, Slave.secondary.timer_red);
            break;
        }
        break;

    default:
        break;
    }
}

void signals_fsm_update()
{
    primary_lamp_fsm_update();
    secondary_lamp_fsm_update();
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
        if (colourID == SlaveState::AUTO_RED)
            return Slave.primary.timer_red;
        else if (colourID == SlaveState::AUTO_AMBER)
            return Slave.primary.timer_amber;
        else if (colourID == SlaveState::AUTO_GREEN)
            return Slave.primary.timer_green;
    }

    else if (lampID == LampID::SECONDARY)
    {
        if (colourID == SlaveState::AUTO_RED)
            return Slave.secondary.timer_red;
        else if (colourID == SlaveState::AUTO_AMBER)
            return Slave.secondary.timer_amber;
        else if (colourID == SlaveState::AUTO_GREEN)
            return Slave.secondary.timer_green;
    }

    else if (lampID == LampID::SPARE)
    {
        if (colourID == SlaveState::AUTO_RED)
            return Slave.spare.timer_red;
        else if (colourID == SlaveState::AUTO_AMBER)
            return Slave.spare.timer_amber;
        else if (colourID == SlaveState::AUTO_GREEN)
            return Slave.spare.timer_green;
    }

    return -1; // if doesn't return anywhere
}

unsigned int getRemainingTime(int lampID)
{
    if (lampID == LampID::PRIMARY || lampID == LampID::OVERHEAD)
    {
        if (Slave.primary.current_state == SlaveState::AUTO_RED)
            return Slave.primary.timer_red - delay_get(Slave.primary.timer_Num);
        else if (Slave.primary.current_state == SlaveState::AUTO_AMBER)
            return Slave.primary.timer_amber - delay_get(Slave.primary.timer_Num);
        else if (Slave.primary.current_state == SlaveState::AUTO_GREEN)
            return Slave.primary.timer_green - delay_get(Slave.primary.timer_Num);
    }

    else if (lampID == LampID::SECONDARY)
    {
        if (Slave.secondary.current_state == SlaveState::AUTO_RED)
            return Slave.secondary.timer_red - delay_get(Slave.secondary.timer_Num);
        else if (Slave.secondary.current_state == SlaveState::AUTO_AMBER)
            return Slave.secondary.timer_amber - delay_get(Slave.secondary.timer_Num);
        else if (Slave.secondary.current_state == SlaveState::AUTO_GREEN)
            return Slave.secondary.timer_green - delay_get(Slave.secondary.timer_Num);
    }

    else if (lampID == LampID::SPARE)
    {
        if (Slave.spare.current_state == SlaveState::AUTO_RED)
            return Slave.spare.timer_red - delay_get(Slave.spare.timer_Num);
        else if (Slave.spare.current_state == SlaveState::AUTO_AMBER)
            return Slave.spare.timer_amber - delay_get(Slave.spare.timer_Num);
        else if (Slave.spare.current_state == SlaveState::AUTO_GREEN)
            return Slave.spare.timer_green - delay_get(Slave.spare.timer_Num);
    }
}

int getMode()
{
    return Traffic.mode;
}

int getTotalSlaves()
{
    return Traffic.n_slaves;
}

void addSlave()
{
    Traffic.n_slaves++;
}

void dropSlave()
{
    Traffic.n_slaves--;
}

void setTotalSlaves(int n)
{
    Traffic.n_slaves = n;
}