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
    int blink_f;   // Frequency (only for blinker mode)

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

    lamp_config config;

public:
    /*PUBLIC METHODS*/
    void moveToState(bool cmd_state_stored = true, int cmd_state = -1)
    {
        if (!cmd_state_stored)
        {
            commanded_state = cmd_state;
        }

        if (current_state == commanded_state)
            return;

        current_state = commanded_state;
        switch (current_state)
        {
        case SlaveState::OFF:
            noTone(amberPin);
            digitalWrite(redPin, 0);
            digitalWrite(amberPin, 0);
            digitalWrite(greenFwdPin, 0);
            digitalWrite(greenLeftPin, 0);
            digitalWrite(greenRightPin, 0);
            break;

        case SlaveState::AUTO_RED:
            noTone(amberPin);
            digitalWrite(redPin, 1);
            digitalWrite(amberPin, 0);
            digitalWrite(greenFwdPin, 0);
            digitalWrite(greenLeftPin, 0);
            digitalWrite(greenRightPin, 0);
            break;

        case SlaveState::AUTO_AMBER:
            noTone(amberPin);
            digitalWrite(redPin, 0);
            digitalWrite(amberPin, 1);
            digitalWrite(greenFwdPin, 0);
            digitalWrite(greenLeftPin, 0);
            digitalWrite(greenRightPin, 0);
            break;

        case SlaveState::AUTO_GREEN:
            digitalWrite(redPin, 0);
            switch (Traffic.mode)
            {
            case MODE_MULTIDIRECTION:
                digitalWrite(greenFwdPin, 1);
                digitalWrite(greenLeftPin, 1);
                digitalWrite(greenRightPin, 1);
                noTone(amberPin);
                break;

            case MODE_STRAIGHT_ONLY:
                digitalWrite(greenFwdPin, 1);
                digitalWrite(greenLeftPin, 0);
                digitalWrite(greenRightPin, 0);
                noTone(amberPin);
                break;
            }
            break;

            /* DICTATED STATES */

        case SlaveState::DICTATED_RED:
            noTone(amberPin);
            digitalWrite(redPin, 1);
            digitalWrite(amberPin, 0);
            digitalWrite(greenFwdPin, 0);
            digitalWrite(greenLeftPin, 0);
            digitalWrite(greenRightPin, 0);
            break;

        case SlaveState::DICTATED_AMBER:
            noTone(amberPin);
            digitalWrite(redPin, 0);
            digitalWrite(amberPin, 1);
            digitalWrite(greenFwdPin, 0);
            digitalWrite(greenLeftPin, 0);
            digitalWrite(greenRightPin, 0);
            break;

        case SlaveState::DICTATED_GREEN:
            digitalWrite(redPin, 0);
            switch (Traffic.mode)
            {
            case MODE_MULTIDIRECTION:
                digitalWrite(greenFwdPin, 1);
                digitalWrite(greenLeftPin, 1);
                digitalWrite(greenRightPin, 1);
                noTone(amberPin);
                break;

            case MODE_STRAIGHT_ONLY:
                digitalWrite(greenFwdPin, 1);
                digitalWrite(greenLeftPin, 0);
                digitalWrite(greenRightPin, 0);
                noTone(amberPin);
                break;
            }
            break;

        case SlaveState::BLINKER:
            tone(amberPin, blink_f);
            digitalWrite(greenFwdPin, 0);
            digitalWrite(greenLeftPin, 0);
            digitalWrite(greenRightPin, 0);
            break;

        default:
            break;
        }
    }

    void lampFsmUpdate()
    {
        int control = getControlMode();
        switch (current_state)
        {
        case SlaveState::OFF:
            if (commanded_state != SlaveState::OFF)
                moveToState();
            break;

        case SlaveState::AUTO_RED:
            if (control == ControlMode::DICTATED)
                moveToState(false, SlaveState::DICTATED_RED);

            if (commanded_state == SlaveState::BLINKER)
                moveToState();

            if (delay_is_done(timer_Num))
            {
                current_state = SlaveState::AUTO_AMBER;
                delay_set(timer_Num, timer_amber);
                break;
            }
            break;

        case SlaveState::AUTO_AMBER:
            if (control == ControlMode::DICTATED)
                moveToState(false, SlaveState::DICTATED_AMBER);

            if (commanded_state == SlaveState::BLINKER)
                moveToState();

            if (delay_is_done(timer_Num))
            {
                current_state = SlaveState::AUTO_GREEN;
                delay_set(timer_Num, timer_green);
                break;
            }
            break;

        case SlaveState::AUTO_GREEN:
            if (control == ControlMode::DICTATED)
                moveToState(false, SlaveState::DICTATED_GREEN);

            if (commanded_state == SlaveState::BLINKER)
                moveToState();

            if (delay_is_done(timer_Num))
            {
                current_state = SlaveState::AUTO_RED;
                delay_set(timer_Num, timer_red);
                break;
            }
            break;

        case SlaveState::DICTATED_RED:
            if (control == ControlMode::AUTO)
                moveToState(false, SlaveState::AUTO_RED);

            if (commanded_state == SlaveState::DICTATED_AMBER)
                moveToState();

            else if (commanded_state == SlaveState::DICTATED_GREEN)
                moveToState();

            else if (commanded_state == SlaveState::BLINKER)
                moveToState();

            break;

        case SlaveState::DICTATED_AMBER:
            if (control == ControlMode::AUTO)
                moveToState(false, SlaveState::AUTO_AMBER);

            if (commanded_state == SlaveState::DICTATED_RED)
                moveToState();

            else if (commanded_state == SlaveState::DICTATED_GREEN)
                moveToState();

            else if (commanded_state == SlaveState::BLINKER)
                moveToState();

            break;

        case SlaveState::DICTATED_GREEN:
            if (control == ControlMode::AUTO)
                moveToState(false, SlaveState::AUTO_GREEN);

            if (commanded_state == SlaveState::DICTATED_AMBER)
                moveToState();

            else if (commanded_state == SlaveState::DICTATED_RED)
                moveToState();

            else if (commanded_state == SlaveState::BLINKER)
                moveToState();

            break;

        case SlaveState::BLINKER:
            // It will loop forever until it has been asked to go to DICTATED_RED (this is basically the red extension part)

            if (commanded_state == SlaveState::DICTATED_RED)
                moveToState();

            break;

        default:
            break;
        }
    }

    void config_lamps(ArduinoJson::JsonObject &obj)
    {
        ArduinoJson::JsonObject &green = obj["green"];
        config.red = obj["red"];
        config.amber = obj["amber"];
        config.green_fwd = green["fwd"];
        config.green_l = green["l"];
        config.green_r = green["r"];
    }

    void init_lamp_config()
    {
        config.red = true;
        config.amber = true;
        config.green_fwd = true;
        config.green_l = true;
        config.green_r = true;
    }

    void init_params_to_zero()
    {
        timer_green = 0;
        timer_red = 0;
        timer_amber = 0;
        blink_f = 0;
    }

    void set_GPIO(int red, int amber, int green_fwd, int green_left, int green_right)
    {
        redPin = red;
        amberPin = amber;
        greenFwdPin = green_fwd;
        greenLeftPin = green_left;
        greenRightPin = green_right;
    }
};

struct slaveStruct
{
public:
    // Slave general configuration info
    int slaveId;
    int oppSlaveId; // ID of opposite slave (-1 if none)
    int panelId;

    lamp primary;
    lamp secondary;
    lamp overhead;
    lamp spare;

} Slave;

struct environmentStruct
{
    int n_slaves;
    int mode;
} Traffic;

/* STATIC METHOD DEFINITONS */
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
    Slave.primary.init_params_to_zero();
    Slave.primary.timer_Num = 0;
    Slave.primary.set_GPIO(PRIMARY_RED,
                           PRIMARY_AMBER,
                           PRIMARY_GREEN_FWD,
                           PRIMARY_GREEN_LEFT,
                           PRIMARY_GREEN_RIGHT);

    /* SECONDARY */
    Slave.secondary.init_params_to_zero();
    Slave.secondary.timer_Num = 1;
    Slave.secondary.set_GPIO(SECONDARY_RED,
                             SECONDARY_AMBER,
                             SECONDARY_GREEN_FWD,
                             SECONDARY_GREEN_LEFT,
                             SECONDARY_GREEN_RIGHT);

    /* Overhead -- this is because overhead is basically the as primary */
    Slave.overhead.init_params_to_zero();
    Slave.overhead.timer_Num = 0;
    Slave.overhead.set_GPIO(OVERHEAD_RED,
                            OVERHEAD_AMBER,
                            OVERHEAD_GREEN_FWD,
                            OVERHEAD_GREEN_LEFT,
                            OVERHEAD_GREEN_RIGHT);

    /* SPARE */
    Slave.spare.init_params_to_zero();
    Slave.spare.timer_Num = 2;
    Slave.spare.set_GPIO(SPARE_RED,
                         SPARE_AMBER,
                         SPARE_GREEN_FWD,
                         SPARE_GREEN_LEFT,
                         SPARE_GREEN_RIGHT);
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
    if (Slave.primary.commanded_state == SlaveState::BLINKER)
    {
        // parse in the blinker frequency
        Slave.primary.blink_f = primary["blink_f"];
    }

    Slave.secondary.commanded_state = secondary["state"];
    Slave.secondary.timer_red = secondary["red"];
    Slave.secondary.timer_green = secondary["green"];
    Slave.secondary.timer_amber = secondary["amber"];
    if (Slave.secondary.commanded_state == SlaveState::BLINKER)
    {
        // parse in the blinker frequency
        Slave.secondary.blink_f = secondary["blink_f"];
    }
}

void initLamp()
{
    // Start every signal in OFF state
    Slave.primary.commanded_state = SlaveState::OFF;
    Slave.secondary.commanded_state = SlaveState::OFF;
    Slave.spare.commanded_state = SlaveState::OFF;

    Slave.primary.moveToState();
    Slave.secondary.moveToState();
}

void signals_fsm_update()
{
    Slave.primary.lampFsmUpdate();
    Slave.secondary.lampFsmUpdate();
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

/* LAMP CONFIG PART */

void signals_config_lamps(ArduinoJson::JsonObject &parsed)
{
    ArduinoJson::JsonObject &config = parsed[(const char *)Slave.slaveId];

    ArduinoJson::JsonObject &primary = config["primary"];
    Slave.primary.config_lamps(primary);

    ArduinoJson::JsonObject &secondary = config["secondary"];
    Slave.secondary.config_lamps(secondary);

    ArduinoJson::JsonObject &overhead = config["overhead"];
    Slave.overhead.config_lamps(overhead);

    ArduinoJson::JsonObject &spare = config["spare"];
    Slave.spare.config_lamps(spare);
}

lamp_config signals_get_lamp_config(LampID id)
{
    switch (id)
    {
    case LampID::PRIMARY:
        return Slave.primary.config;

    case LampID::SECONDARY:
        return Slave.secondary.config;

    case LampID::OVERHEAD:
        return Slave.overhead.config;

    case LampID::SPARE:
        return Slave.spare.config;

    default:
        break;
    }
}

void signals_init_lamp_config()
{
    Slave.primary.init_lamp_config();
    Slave.secondary.init_lamp_config();
    Slave.overhead.init_lamp_config();
    Slave.spare.init_lamp_config();
}
