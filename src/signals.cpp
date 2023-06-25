#include "signals.h"
#include "delay.h"
#include "Arduino.h"
#include "mqtt.h"
#include "timer.h"
#include "configurations.h"

/*
#define SLAVE_STATE_RED 0
#define SLAVE_STATE_YELLOW 2
#define SLAVE_STATE_GREEN 1
*/

int signal_state;
int mode;

void signals_start()
{
    // Start with red
    signal_state = SLAVE_STATE_RED;
}

void signals_set_state(int state)
{
    // Set the state of the fsm
    signal_state = state;
}

void signals_set_mode(int mode_select)
{
    mode = mode;
}

void signals_off()
{
    digitalWrite(RED, 0);
    digitalWrite(GREEN_FWD, 0);
    digitalWrite(GREEN_LEFT, 0);
    digitalWrite(GREEN_RIGHT, 0);
}

void signals_red(int val)
{
    digitalWrite(RED, val);
}

void signals_green_forward(int val)
{
    digitalWrite(GREEN_FWD, val);
}

void signals_green_left(int val)
{
    digitalWrite(GREEN_LEFT, val);
}

void signals_green_right(int val)
{
    digitalWrite(GREEN_RIGHT, val);
}

void signals_update()
{
    switch (signal_state)
    {
    case SLAVE_STATE_RED:
        // Turn ON red LED
        digitalWrite(RED, 1);

        // Turn OFF all greens
        digitalWrite(GREEN_FWD, 0);
        digitalWrite(GREEN_LEFT, 0);
        digitalWrite(GREEN_RIGHT, 0);
        if (delay_is_done(0))
        {
            signal_state = SLAVE_STATE_GREEN;
            timer_start(SLAVE_STATE_GREEN);
            return;
        }
        break;

    case SLAVE_STATE_GREEN:
        // Turn ON greens
        digitalWrite(GREEN_FWD, 1);
        if (mode == MODE_MULTIDIRECTION)
        {
            digitalWrite(GREEN_LEFT, 1);
            digitalWrite(GREEN_RIGHT, 1);
        }

        // Turn OFF red
        digitalWrite(RED, 0);

        if (delay_is_done(0))
        {
            signal_state = SLAVE_STATE_RED;
            // delay_set(0, 1);  //delay_set(0, GREEN_TIMER)
            timer_start(SLAVE_STATE_RED);
            return;
        }
        break;

    default:
        break;
    }
}

int signals_get_state()
{
    return signal_state;
}