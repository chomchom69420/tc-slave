// #include "Arduino.h"
#include "lamp_checker.h"

uint8_t colour_sel;
float i_on_thresh;
float i_overcurrent_lim;

void lamp_checker_init()
{
    pinMode(CURRENT_SENSE_SEL0, OUTPUT);
    pinMode(CURRENT_SENSE_SEL1, OUTPUT);
    pinMode(CURRENT_SENSE_SEL2, OUTPUT);

    // Set the select pins to 0
    colour_sel = 0;

    // Set i_on_thresh to 1 A
    i_on_thresh = 1;

    // Set overcurrent limit to 3.5 A
    i_overcurrent_lim = 3.5;
}

static int read_adc(Current_Sense_Input input)
{
    return analogRead(input);
}

static void set_select_pins()
{
    digitalWrite(Current_Sense_Sel::SEL0, colour_sel & (1 << 0));
    digitalWrite(Current_Sense_Sel::SEL1, colour_sel & (1 << 1));
    digitalWrite(Current_Sense_Sel::SEL2, colour_sel & (1 << 2));
}

static float adc_to_current(int raw)
{
    // Insert callibration stuff here
    return raw;
}

static float get_current(Current_Sense_Input pin, Lamp_Channel colour_channel)
{
    colour_sel = colour_channel;
    set_select_pins();
    return adc_to_current(read_adc(pin));
}

bool get_lamp_status(LampID id, Lamp_Channel colour_channel)
{
    Current_Sense_Input pin;
    if (id == LampID::PRIMARY)
        pin = Current_Sense_Input::ISENSE_PRIMARY;
    else if (id == LampID::SECONDARY)
        pin = Current_Sense_Input::ISENSE_SECONDARY;
    else if (id == LampID::OVERHEAD)
        pin = Current_Sense_Input::ISENSE_OVERHEAD;
    else if (id == LampID::SPARE)
        pin = Current_Sense_Input::ISENSE_SPARE;

    return get_current(pin, colour_channel) > i_on_thresh;
}

void log_lamp_status()
{
    char payload[700];

    const size_t capacity = 5 * JSON_OBJECT_SIZE(5) + 240;
    DynamicJsonBuffer jsonBuffer(capacity);
    JsonObject &root = jsonBuffer.createObject();

    root["slave_id"] = SLAVE_ID;

    JsonObject &primary = root.createNestedObject("primary");
    primary["red"] = get_lamp_status(LampID::PRIMARY, Lamp_Channel::RED);
    primary["amber"] = get_lamp_status(LampID::PRIMARY, Lamp_Channel::AMBER);
    primary["g_fwd"] = get_lamp_status(LampID::PRIMARY, Lamp_Channel::GREEN_FWD);
    primary["g_left"] = get_lamp_status(LampID::PRIMARY, Lamp_Channel::GREEN_LEFT);
    primary["g_right"] = get_lamp_status(LampID::PRIMARY, Lamp_Channel::GREEN_RIGHT);

    JsonObject &secondary = root.createNestedObject("secondary");
    secondary["red"] = get_lamp_status(LampID::SECONDARY, Lamp_Channel::RED);
    secondary["amber"] = get_lamp_status(LampID::SECONDARY, Lamp_Channel::AMBER);
    secondary["g_fwd"] = get_lamp_status(LampID::SECONDARY, Lamp_Channel::GREEN_FWD);
    secondary["g_left"] = get_lamp_status(LampID::SECONDARY, Lamp_Channel::GREEN_LEFT);
    secondary["g_right"] = get_lamp_status(LampID::SECONDARY, Lamp_Channel::GREEN_RIGHT);

    JsonObject &spare = root.createNestedObject("spare");
    spare["red"] = get_lamp_status(LampID::SPARE, Lamp_Channel::RED);
    spare["amber"] = get_lamp_status(LampID::SPARE, Lamp_Channel::AMBER);
    spare["g_fwd"] = get_lamp_status(LampID::SPARE, Lamp_Channel::GREEN_FWD);
    spare["g_left"] = get_lamp_status(LampID::SPARE, Lamp_Channel::GREEN_LEFT);
    spare["g_right"] = get_lamp_status(LampID::SPARE, Lamp_Channel::GREEN_RIGHT);

    JsonObject &overhead = root.createNestedObject("overhead");
    overhead["red"] = get_lamp_status(LampID::OVERHEAD, Lamp_Channel::RED);
    overhead["amber"] = get_lamp_status(LampID::OVERHEAD, Lamp_Channel::AMBER);
    overhead["g_fwd"] = get_lamp_status(LampID::OVERHEAD, Lamp_Channel::GREEN_FWD);
    overhead["g_left"] = get_lamp_status(LampID::OVERHEAD, Lamp_Channel::GREEN_LEFT);
    overhead["g_right"] = get_lamp_status(LampID::OVERHEAD, Lamp_Channel::GREEN_RIGHT);

    root.printTo(payload);
    log(payload, "/traffic/lamp_status");
}

void set_thresh_current(float i)
{
    i_on_thresh = i;
}

float get_thresh_current()
{
    return i_on_thresh;
}

void set_overcurrent_limit(float i)
{
    i_overcurrent_lim = i;
}

float get_overcurrent_limit()
{
    return i_overcurrent_lim;
}

int health_code(LampID id, Lamp_Channel channel)
{

    Current_Sense_Input pin;
    if (id == LampID::PRIMARY)
        pin = Current_Sense_Input::ISENSE_PRIMARY;
    else if (id == LampID::SECONDARY)
        pin = Current_Sense_Input::ISENSE_SECONDARY;
    else if (id == LampID::OVERHEAD)
        pin = Current_Sense_Input::ISENSE_OVERHEAD;
    else if (id == LampID::SPARE)
        pin = Current_Sense_Input::ISENSE_SPARE;

    if (get_current(pin, channel) > i_overcurrent_lim)
        return 2;

    if (get_lamp_enable(id, channel))
    {
        if (signals_get_lamp_status(id, channel) == get_lamp_status(id, channel))
            return 0;
        else
            return 1;
    }
    else
    {
        if (get_lamp_status(id, channel))
            return 1;
        else
            return 0;
    }
}

void lamp_checker_log_health()
{
    char payload[700];

    const char* payload_ptr = payload;

    const size_t capacity = 5 * JSON_OBJECT_SIZE(5) + 240;
    DynamicJsonBuffer jsonBuffer(capacity);
    JsonObject &root = jsonBuffer.createObject();

    root["slave_id"] = SLAVE_ID;

    JsonObject &primary = root.createNestedObject("primary");
    primary["red"] = health_code(LampID::PRIMARY, Lamp_Channel::RED);
    primary["amber"] = health_code(LampID::PRIMARY, Lamp_Channel::AMBER);
    primary["g_fwd"] = health_code(LampID::PRIMARY, Lamp_Channel::GREEN_FWD);
    primary["g_left"] = health_code(LampID::PRIMARY, Lamp_Channel::GREEN_LEFT);
    primary["g_right"] = health_code(LampID::PRIMARY, Lamp_Channel::GREEN_RIGHT);

    JsonObject &secondary = root.createNestedObject("secondary");
    secondary["red"] = health_code(LampID::SECONDARY, Lamp_Channel::RED);
    secondary["amber"] = health_code(LampID::SECONDARY, Lamp_Channel::AMBER);
    secondary["g_fwd"] = health_code(LampID::SECONDARY, Lamp_Channel::GREEN_FWD);
    secondary["g_left"] = health_code(LampID::SECONDARY, Lamp_Channel::GREEN_LEFT);
    secondary["g_right"] = health_code(LampID::SECONDARY, Lamp_Channel::GREEN_RIGHT);

    JsonObject &spare = root.createNestedObject("spare");
    spare["red"] = health_code(LampID::SPARE, Lamp_Channel::RED);
    spare["amber"] = health_code(LampID::SPARE, Lamp_Channel::AMBER);
    spare["g_fwd"] = health_code(LampID::SPARE, Lamp_Channel::GREEN_FWD);
    spare["g_left"] = health_code(LampID::SPARE, Lamp_Channel::GREEN_LEFT);
    spare["g_right"] = health_code(LampID::SPARE, Lamp_Channel::GREEN_RIGHT);

    JsonObject &overhead = root.createNestedObject("overhead");
    overhead["red"] = health_code(LampID::OVERHEAD, Lamp_Channel::RED);
    overhead["amber"] = health_code(LampID::OVERHEAD, Lamp_Channel::AMBER);
    overhead["g_fwd"] = health_code(LampID::OVERHEAD, Lamp_Channel::GREEN_FWD);
    overhead["g_left"] = health_code(LampID::OVERHEAD, Lamp_Channel::GREEN_LEFT);
    overhead["g_right"] = health_code(LampID::OVERHEAD, Lamp_Channel::GREEN_RIGHT);

    root.printTo(payload);
    const char* topic = "/traffic/monitoring";
    log(topic, payload_ptr);
}
