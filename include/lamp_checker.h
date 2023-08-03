/*
This api:
- interfaces with the current sensor chip on each lamp channel
- code to check if actual lamp state matches desired lamp state for each primary, secondary, overhead and spare 
- code to perform lamp-by-lamp check (maybe on startup) to see if any lamp is not working
*/

#include "configurations.h"

/*
Sets the MUX selector pins as output and sets up analog read

Sets the on threshold current to 1 A
Sets the overcurrent limit to 3.5 A

*/
void lamp_checker_init();

/*
Get the status of a specific lamp
enum LampID
{
    PRIMARY = 0,
    SECONDARY = 1,
    OVERHEAD = 2,
    SPARE = 3
};

enum Lamp_Colour{
    RED=0,
    AMBER=1,
    GREEN=2
};

Return values: 
0 --> lamp is off (current = 0)
1 --> lamp is on (current > thresh_current)

*/
bool get_lamp_status(LampID id, Lamp_Channel colour_channel);

/*
Logs the lamp status in an mqtt message
Topic: /traffic/lamp_status
JSON payload format:
{
    "slave_id": ,
    "primary": {
        "red": 0,
        "amber": 1,
        "g_fwd": 0,
        "g_left": 1,
        "g_right": 0
    },
    "secondary": {
        "red": 0,
        "amber": 1,
        "g_fwd": 0,
        "g_left": 1,
        "g_right": 0
    },
    "overhead": {
        "red": 0,
        "amber": 1,
        "g_fwd": 0,
        "g_left": 1,
        "g_right": 0
    },
    "spare": {
        "red": 0,
        "amber": 1,
        "g_fwd": 0,
        "g_left": 1,
        "g_right": 0
    }
}
*/
void log_lamp_status();

/*
Checks the lamp states against the desired states and logs the results in an MQTT message
Topic: /traffic/monitoring
JSON payload format:
{
    "slave_id": ,
    "primary": {
        "red": 0,
        "amber": 1,
        "g_fwd": 0,
        "g_left": 1,
        "g_right": 0
    },
    "secondary": {
        "red": 0,
        "amber": 1,
        "g_fwd": 0,
        "g_left": 1,
        "g_right": 0
    },
    "overhead": {
        "red": 0,
        "amber": 1,
        "g_fwd": 0,
        "g_left": 1,
        "g_right": 0
    },
    "spare": {
        "red": 0,
        "amber": 1,
        "g_fwd": 0,
        "g_left": 1,
        "g_right": 0
    }
}
*/
void lamp_checker_log_health();

/*
Returns an error code:
0 → healthy 
This means that the Lamp is OFF and the intended state is OFF or disabled. 

1 → error
This means that the actual state is not the same as the desired state. ON / OFF or OFF / ON

2 → overcurrent
This means that the current is greater than the overcurrent limit
*/
int health_code(LampID id, Lamp_Channel colour_channel);

/*
Sets the threshold current for declaring the lamp to be ON
*/
void set_thresh_current(float i);

/*
Returns the threshold current for declaring the lamp to be ON
*/
float get_thresh_current();

/*
Sets the overcurrent limit for declaring the lamp
*/
void set_overcurrent_limit(float i);

/*
Returns the overcurrent limit for declaring the lamp
*/
float get_overcurrent_limit();
