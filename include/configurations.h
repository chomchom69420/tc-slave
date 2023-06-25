// #define SLAVE_ID 1
// #define SLAVE_ID 2
#define SLAVE_ID 3
// #define SLAVE_ID 4
// #define SLAVE_ID 5
// #define SLAVE_ID 6
// #define SLAVE_ID 7


//Pinouts - WROOM
#define WROOM_RED 32
#define WROOM_AMBER 33
#define WROOM_GREEN_FORWARD 25
#define WROOM_GREEN_LEFT 26 
#define WROOM_GREEN_RIGHT 27

//Pinouts - Adafruit Feather ESP32
// --- Not needed yet --- //

#define SLAVE_STATE_RED 0
#define SLAVE_STATE_GREEN 1

typedef struct {
    int n_slaves;
    int states[7];
} SlaveStates;

// Mode select
#define MODE_STRAIGHT_ONLY 1
#define MODE_MULTIDIRECTION 2


//Pinouts 
#define RED 13
#define AMBER 12
#define GREEN_FWD 27
#define GREEN_LEFT 33
#define GREEN_RIGHT 15





