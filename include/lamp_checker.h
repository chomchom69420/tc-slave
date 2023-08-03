/*
This api:
- interfaces with the current sensor chip on each lamp channel
- code to check if actual lamp state matches desired lamp state for each primary, secondary, overhead and spare 
- code to perform lamp-by-lamp check (maybe on startup) to see if any lamp is not working
*/

#include "configurations.h"

/*
Sets the MUX selector pins as output and sets up analog read

Sets the threshold current to  A

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

void log_lamp_status();

/*
Sets the threshold current for declaring the lamp to be ON
*/
void set_thresh_current(int i);

/*
Sets the threshold current for declaring the lamp to be ON
*/
void get_thresh_current(int i);

