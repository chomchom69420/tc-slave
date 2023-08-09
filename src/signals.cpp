#include "signals.h"
#include "signals_mqtt.h"

typedef struct lamp_states
{
    bool red;
    bool amber;
    bool g_fwd;
    bool g_left;
    bool g_right;
};

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
    lamp_states states;

public:
    /*PUBLIC METHODS*/

    void signals_move_to_state(bool cmd_state_stored, int cmd_state, int mode)
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
            states.red = 0;
            states.amber = 0;
            states.g_fwd = 0;
            states.g_left = 0;
            states.g_right = 0;
            set_GPIO();

            break;

        case SlaveState::AUTO_RED:
            noTone(amberPin);
            states.red = 1;
            states.amber = 0;
            states.g_fwd = 0;
            states.g_left = 0;
            states.g_right = 0;
            set_GPIO();
            break;

        case SlaveState::AUTO_AMBER:
            noTone(amberPin);
            states.red = 0;
            states.amber = 1;
            states.g_fwd = 0;
            states.g_left = 0;
            states.g_right = 0;
            set_GPIO();
            break;

        case SlaveState::AUTO_GREEN:
            switch (mode)
            {
            case MODE_MULTIDIRECTION:
                states.red = 0;
                states.amber = 0;
                states.g_fwd = 1;
                states.g_left = 1;
                states.g_right = 1;
                set_GPIO();
                noTone(amberPin);
                break;

            case MODE_STRAIGHT_ONLY:
                states.red = 0;
                states.amber = 0;
                states.g_fwd = 1;
                states.g_left = 0;
                states.g_right = 0;
                set_GPIO();
                noTone(amberPin);
                break;
            }
            break;

            /* DICTATED STATES */

        case SlaveState::DICTATED_RED:
            noTone(amberPin);
            states.red = 1;
            states.amber = 0;
            states.g_fwd = 0;
            states.g_left = 0;
            states.g_right = 0;
            set_GPIO();
            break;

        case SlaveState::DICTATED_AMBER:
            noTone(amberPin);
            states.red = 0;
            states.amber = 1;
            states.g_fwd = 0;
            states.g_left = 0;
            states.g_right = 0;
            set_GPIO();
            break;

        case SlaveState::DICTATED_GREEN:
            switch (mode)
            {
            case MODE_MULTIDIRECTION:
                states.red = 0;
                states.amber = 0;
                states.g_fwd = 1;
                states.g_left = 1;
                states.g_right = 1;
                set_GPIO();
                noTone(amberPin);
                break;

            case MODE_STRAIGHT_ONLY:
                states.red = 0;
                states.amber = 0;
                states.g_fwd = 1;
                states.g_left = 0;
                states.g_right = 0;
                set_GPIO();
                noTone(amberPin);
                break;
            }
            break;

        case SlaveState::BLINKER:
            states.red = 0;
            states.amber = 0;
            states.g_fwd = 0;
            states.g_left = 0;
            states.g_right = 0;
            set_GPIO();
            tone(amberPin, blink_f);

            break;

        default:
            break;
        }
    }

    void lampFsmUpdate(int mode)
    {
        int control = getControlMode();
        switch (current_state)
        {
        case SlaveState::OFF:
            if (commanded_state != SlaveState::OFF)
                signals_move_to_state(true, -1, mode);
            break;

        case SlaveState::AUTO_RED:
            if (control == ControlMode::DICTATED)
                signals_move_to_state(false, SlaveState::DICTATED_RED, mode);

            if (commanded_state == SlaveState::BLINKER)
                signals_move_to_state(true, -1, mode);

            if (delay_is_done(timer_Num))
            {
                current_state = SlaveState::AUTO_AMBER;
                delay_set(timer_Num, timer_amber);
                break;
            }
            break;

        case SlaveState::AUTO_AMBER:
            if (control == ControlMode::DICTATED)
                signals_move_to_state(false, SlaveState::DICTATED_AMBER, mode);

            if (commanded_state == SlaveState::BLINKER)
                signals_move_to_state(true, -1, mode);

            if (delay_is_done(timer_Num))
            {
                current_state = SlaveState::AUTO_GREEN;
                delay_set(timer_Num, timer_green);
                break;
            }
            break;

        case SlaveState::AUTO_GREEN:
            if (control == ControlMode::DICTATED)
                signals_move_to_state(false, SlaveState::DICTATED_GREEN, mode);

            if (commanded_state == SlaveState::BLINKER)
                signals_move_to_state(true, -1, mode);

            if (delay_is_done(timer_Num))
            {
                current_state = SlaveState::AUTO_RED;
                delay_set(timer_Num, timer_red);
                break;
            }
            break;

        case SlaveState::DICTATED_RED:
            if (control == ControlMode::AUTO)
                signals_move_to_state(false, SlaveState::AUTO_RED, mode);

            if (commanded_state == SlaveState::DICTATED_AMBER)
                signals_move_to_state(true, -1, mode);

            else if (commanded_state == SlaveState::DICTATED_GREEN)
                signals_move_to_state(true, -1, mode);

            else if (commanded_state == SlaveState::BLINKER)
                signals_move_to_state(true, -1, mode);

            break;

        case SlaveState::DICTATED_AMBER:
            if (control == ControlMode::AUTO)
                signals_move_to_state(false, SlaveState::AUTO_AMBER,mode);

            if (commanded_state == SlaveState::DICTATED_RED)
                signals_move_to_state(true, -1, mode);

            else if (commanded_state == SlaveState::DICTATED_GREEN)
                signals_move_to_state(true, -1, mode);

            else if (commanded_state == SlaveState::BLINKER)
                signals_move_to_state(true, -1, mode);

            break;

        case SlaveState::DICTATED_GREEN:
            if (control == ControlMode::AUTO)
                signals_move_to_state(false, SlaveState::AUTO_GREEN, mode);

            if (commanded_state == SlaveState::DICTATED_AMBER, mode)
                signals_move_to_state(true, -1, mode);

            else if (commanded_state == SlaveState::DICTATED_RED, mode)
                signals_move_to_state(true, -1, mode);

            else if (commanded_state == SlaveState::BLINKER)
                signals_move_to_state(true, -1, mode);

            break;

        case SlaveState::BLINKER:
            // It will loop forever until it has been asked to go to DICTATED_RED (this is basically the red extension part)

            if (commanded_state == SlaveState::DICTATED_RED)
                signals_move_to_state(true, -1, mode);

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

    void set_GPIO()
    {
        digitalWrite(redPin, states.red);
        digitalWrite(amberPin, states.amber);
        digitalWrite(greenFwdPin, states.g_fwd);
        digitalWrite(greenLeftPin, states.g_left);
        digitalWrite(greenRightPin, states.g_right);
    }

    void set_GPIO_Pins(int rPin, int ambPin, int gFwdPin, int gLeftPin, int gRightPin)
    {
        redPin = rPin;
        amberPin = ambPin;
        greenFwdPin = gFwdPin;
        greenLeftPin = gLeftPin;
        greenRightPin = gRightPin;
        pinMode(redPin, OUTPUT);
        pinMode(amberPin, OUTPUT);
        pinMode(greenFwdPin, OUTPUT);
        pinMode(greenLeftPin, OUTPUT);
        pinMode(greenRightPin, OUTPUT);
    }
};

struct slaveStruct
{
public:
    // Slave general configuration info
    int slaveId;
    int oppSlaveId; // ID of opposite slave (-1 if none)

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

/* STATIC METHODS */
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

void signals_init_environment()
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

void signals_init_slave()
{
    // This function is used to initialize the struct Slave
    Slave.slaveId = SLAVE_ID;
    setOppositeSlaveID();

    /* PRIMARY */
    Slave.primary.init_params_to_zero();
    Slave.primary.timer_Num = 0;
    Slave.primary.set_GPIO_Pins(PRIMARY_RED,
                           PRIMARY_AMBER,
                           PRIMARY_GREEN_FWD,
                           PRIMARY_GREEN_LEFT,
                           PRIMARY_GREEN_RIGHT);

    /* SECONDARY */
    Slave.secondary.init_params_to_zero();
    Slave.secondary.timer_Num = 1;
    Slave.secondary.set_GPIO_Pins(SECONDARY_RED,
                             SECONDARY_AMBER,
                             SECONDARY_GREEN_FWD,
                             SECONDARY_GREEN_LEFT,
                             SECONDARY_GREEN_RIGHT);

    /* Overhead -- this is because overhead is basically the as primary */
    Slave.overhead.init_params_to_zero();
    Slave.overhead.timer_Num = 0;
    Slave.overhead.set_GPIO_Pins(OVERHEAD_RED,
                            OVERHEAD_AMBER,
                            OVERHEAD_GREEN_FWD,
                            OVERHEAD_GREEN_LEFT,
                            OVERHEAD_GREEN_RIGHT);

    /* SPARE */
    Slave.spare.init_params_to_zero();
    Slave.spare.timer_Num = 2;
    Slave.spare.set_GPIO_Pins(SPARE_RED,
                         SPARE_AMBER,
                         SPARE_GREEN_FWD,
                         SPARE_GREEN_LEFT,
                         SPARE_GREEN_RIGHT);
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

void signals_init_lamp()
{
    // Start every signal in OFF state
    Slave.primary.commanded_state = SlaveState::OFF;
    Slave.secondary.commanded_state = SlaveState::OFF;
    Slave.spare.commanded_state = SlaveState::OFF;

    Slave.primary.signals_move_to_state(true, -1, Traffic.mode);
    Slave.secondary.signals_move_to_state(true, -1, Traffic.mode);
}

void signals_fsm_update()
{
    Slave.primary.lampFsmUpdate(Traffic.mode);
    Slave.secondary.lampFsmUpdate(Traffic.mode);
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
    else    // else if (lampID == LampID::SPARE)
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

int signals_get_mode()
{
    return Traffic.mode;
}

int signals_get_total_slaves()
{
    return Traffic.n_slaves;
}

void signals_add_slave()
{
    Traffic.n_slaves++;
}

void signals_drop_slave()
{
    Traffic.n_slaves--;
}

void signals_set_total_slaves(int n)
{
    Traffic.n_slaves = n;
}

bool signals_get_lamp_status(LampID id, Lamp_Channel channel)
{
    switch (id)
    {
    case LampID::PRIMARY:
        switch (channel)
        {
        case Lamp_Channel::RED:
            return Slave.primary.states.red;
        case Lamp_Channel::AMBER:
            return Slave.primary.states.amber;
        case Lamp_Channel::GREEN_FWD:
            return Slave.primary.states.g_fwd;
        case Lamp_Channel::GREEN_LEFT:
            return Slave.primary.states.g_left;
        case Lamp_Channel::GREEN_RIGHT:
            return Slave.primary.states.g_right;
        }

    case LampID::SECONDARY:
        switch (channel)
        {
        case Lamp_Channel::RED:
            return Slave.secondary.states.red;
        case Lamp_Channel::AMBER:
            return Slave.secondary.states.amber;
        case Lamp_Channel::GREEN_FWD:
            return Slave.secondary.states.g_fwd;
        case Lamp_Channel::GREEN_LEFT:
            return Slave.secondary.states.g_left;
        case Lamp_Channel::GREEN_RIGHT:
            return Slave.secondary.states.g_right;
        }
    case LampID::OVERHEAD:
        switch (channel)
        {
        case Lamp_Channel::RED:
            return Slave.overhead.states.red;
        case Lamp_Channel::AMBER:
            return Slave.overhead.states.amber;
        case Lamp_Channel::GREEN_FWD:
            return Slave.overhead.states.g_fwd;
        case Lamp_Channel::GREEN_LEFT:
            return Slave.overhead.states.g_left;
        case Lamp_Channel::GREEN_RIGHT:
            return Slave.overhead.states.g_right;
        }
    case LampID::SPARE:
        switch (channel)
        {
        case Lamp_Channel::RED:
            return Slave.spare.states.red;
        case Lamp_Channel::AMBER:
            return Slave.spare.states.amber;
        case Lamp_Channel::GREEN_FWD:
            return Slave.spare.states.g_fwd;
        case Lamp_Channel::GREEN_LEFT:
            return Slave.spare.states.g_left;
        case Lamp_Channel::GREEN_RIGHT:
            return Slave.spare.states.g_right;
        }

    default:
        break;
    }
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

bool get_lamp_enable(LampID id, Lamp_Channel channel)
{
    switch (id)
    {
    case LampID::PRIMARY:
        switch (channel)
        {
        case Lamp_Channel::RED:
            return Slave.primary.config.red;
        case Lamp_Channel::AMBER:
            return Slave.primary.config.amber;
        case Lamp_Channel::GREEN_FWD:
            return Slave.primary.config.green_fwd;
        case Lamp_Channel::GREEN_LEFT:
            return Slave.primary.config.green_l;
        case Lamp_Channel::GREEN_RIGHT:
            return Slave.primary.config.green_r;
        }

    case LampID::SECONDARY:
        switch (channel)
        {
        case Lamp_Channel::RED:
            return Slave.secondary.config.red;
        case Lamp_Channel::AMBER:
            return Slave.secondary.config.amber;
        case Lamp_Channel::GREEN_FWD:
            return Slave.secondary.config.green_fwd;
        case Lamp_Channel::GREEN_LEFT:
            return Slave.secondary.config.green_l;
        case Lamp_Channel::GREEN_RIGHT:
            return Slave.secondary.config.green_r;
        }

    case LampID::OVERHEAD:
        switch (channel)
        {
        case Lamp_Channel::RED:
            return Slave.overhead.config.red;
        case Lamp_Channel::AMBER:
            return Slave.overhead.config.amber;
        case Lamp_Channel::GREEN_FWD:
            return Slave.overhead.config.green_fwd;
        case Lamp_Channel::GREEN_LEFT:
            return Slave.overhead.config.green_l;
        case Lamp_Channel::GREEN_RIGHT:
            return Slave.overhead.config.green_r;
        }

    case LampID::SPARE:
        switch (channel)
        {
        case Lamp_Channel::RED:
            return Slave.spare.config.red;
        case Lamp_Channel::AMBER:
            return Slave.spare.config.amber;
        case Lamp_Channel::GREEN_FWD:
            return Slave.spare.config.green_fwd;
        case Lamp_Channel::GREEN_LEFT:
            return Slave.spare.config.green_l;
        case Lamp_Channel::GREEN_RIGHT:
            return Slave.spare.config.green_r;
        }

    default:
        break;
    }
}