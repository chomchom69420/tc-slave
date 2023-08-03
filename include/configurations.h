// #define SLAVE_ID 1
// #define SLAVE_ID 2
// #define SLAVE_ID 3
#define SLAVE_ID 4
// #define SLAVE_ID 5
// #define SLAVE_ID 6
// #define SLAVE_ID 7

enum LampID
{
    PRIMARY = 0,
    SECONDARY = 1,
    OVERHEAD = 2,
    SPARE = 3
};

enum Lamp_Channel{
    RED=0,
    AMBER=1,
    GREEN_FWD=2,
    GREEN_LEFT=3,
    GREEN_RIGHT=4
};

enum SlaveState
{
    OFF,
    AUTO_RED,
    AUTO_AMBER,
    AUTO_GREEN,
    DICTATED_RED,
    DICTATED_AMBER,
    DICTATED_GREEN, 
    BLINKER
};

#define PANEL_ID 20

//Pinouts - WROOM
// --- Not used --- //
#define WROOM_RED 32
#define WROOM_AMBER 33
#define WROOM_GREEN_FORWARD 25
#define WROOM_GREEN_LEFT 26 
#define WROOM_GREEN_RIGHT 27

//Pinouts - Adafruit Feather ESP32
// --- Not needed yet --- //

#define SLAVE_STATE_IDLE 0
#define SLAVE_STATE_RED 1
#define SLAVE_STATE_AMBER 2
#define SLAVE_STATE_GREEN 3


// Mode select
#define MODE_STRAIGHT_ONLY 1
#define MODE_MULTIDIRECTION 2
#define MODE_BLINKER 3

//Pinouts 
#define PRIMARY_RED 13
#define PRIMARY_AMBER 12
#define PRIMARY_GREEN_FWD 14
#define PRIMARY_GREEN_LEFT 27
#define PRIMARY_GREEN_RIGHT 26

#define SECONDARY_RED 25
#define SECONDARY_AMBER 15
#define SECONDARY_GREEN_FWD 2
#define SECONDARY_GREEN_LEFT 4
#define SECONDARY_GREEN_RIGHT 16

#define OVERHEAD_RED 13
#define OVERHEAD_AMBER 12
#define OVERHEAD_GREEN_FWD 14
#define OVERHEAD_GREEN_LEFT 27
#define OVERHEAD_GREEN_RIGHT 26

#define SPARE_RED 17
#define SPARE_AMBER 5
#define SPARE_GREEN_FWD 18
#define SPARE_GREEN_LEFT 19
#define SPARE_GREEN_RIGHT 21

enum Current_Sense_Sel {
    SEL0 = 1,
    SEL1 = 22,
    SEL2 = 23
};

#define CURRENT_SENSE_SEL0 1
#define CURRENT_SENSE_SEL1 22
#define CURRENT_SENSE_SEL2 23

enum Current_Sense_Input {
    PRIMARY = 33,
    SECONDARY = 32,
    OVERHEAD = 35,
    SPARE = 34
};

#define PRIMARY_CURRENT_SENSE_IN 33
#define SECONDARY_CURRENT_SENSE_IN 32
#define OVERHEAD_CURRENT_SENSE_IN 35
#define SPARE_CURRENT_SENSE_IN 34







