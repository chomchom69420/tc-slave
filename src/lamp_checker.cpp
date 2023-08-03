#include "Arduino.h"
#include "lamp_checker.h"
#include "mqtt.h"

uint8_t colour_sel;
float i_thresh;

void lamp_checker_init() {
    pinMode(CURRENT_SENSE_SEL0, OUTPUT);
    pinMode(CURRENT_SENSE_SEL1, OUTPUT);
    pinMode(CURRENT_SENSE_SEL2, OUTPUT);

    //Set the select pins to 0
    colour_sel = 0;
    
    //Set i_thresh to 1 A
    i_thresh = 1;
}

static int read_adc(Current_Sense_Input input)
{
    return analogRead(input);
}

static int set_select_pins() {
    digitalWrite(Current_Sense_Sel::SEL0, colour_sel & (1 << 0));
    digitalWrite(Current_Sense_Sel::SEL1, colour_sel & (1 << 1));
    digitalWrite(Current_Sense_Sel::SEL2, colour_sel & (1 << 2));      
}

static float adc_to_current(int raw)
{
    //Insert callibration stuff here
    return raw;
}

static float get_current(Current_Sense_Input pin, Lamp_Channel colour_channel)
{
    colour_sel = colour_channel;
    set_select_pins();
    return adc_to_current(read_adc(pin));
}

bool get_lamp_status(LampID id, Lamp_Channel colour_channel) {
    Current_Sense_Input pin;
    if(id == LampID::PRIMARY)
        pin = Current_Sense_Input::PRIMARY;
    else if(id == LampID::SECONDARY)
        pin = Current_Sense_Input::SECONDARY;
    else if(id == LampID::OVERHEAD)
        pin = Current_Sense_Input::OVERHEAD;
    else if(id == LampID::SPARE)
        pin = Current_Sense_Input::SPARE;

    return get_current(pin, colour_channel) > i_thresh;
}

