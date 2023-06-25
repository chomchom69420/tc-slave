#include "delay.h"
#include "Arduino.h"

static volatile unsigned int delay_count[2] = {0, 0};
static unsigned int limit[2] = {0, 0};
static unsigned char initialized = 0;

hw_timer_t* signal_timer = NULL;

void ARDUINO_ISR_ATTR onTimer(){
  // Increment the counters
  for (unsigned int i = 0; i < sizeof(delay_count) / sizeof(delay_count[0]); i++) {
        if (delay_count[i] < limit[i]) {
            delay_count[i]++;
        }
    }
}

void delay_init(){
    if(!initialized)
    {
        //Initialize hardware timer config 

        /*
        timer 0
        prescaler 80
        countUp true
        */
        signal_timer = timerBegin(0, 80, true);

        //Attach interrupt function to the timer
        timerAttachInterrupt(signal_timer, &onTimer, true);

        //Set timer alarm
        /*
        timer struct: signal_timer
        alarm_value: 1000000 (1s)
        autoreload: true
        */
        timerAlarmWrite(signal_timer, 1000000, true); 

        //Start alarm
        timerAlarmEnable(signal_timer);

        //Set initialized to prevent further initialization
        initialized=1;
    }
}


unsigned int delay_get(unsigned int num) {

    unsigned int count_value;

    //Disable alarm
    timerAlarmDisable(signal_timer);

    //Read counter value 
    count_value = delay_count[num];

    //Enable alarm
    timerAlarmEnable(signal_timer);

    // Return the count value
    return count_value;
}

void delay_set(unsigned int num, unsigned int time) {

    // If not initialized, initialize the delay counter
    if (!initialized) {
        delay_init();
    }
    
    //Disable timer alarm (interrupts)
    timerAlarmDisable(signal_timer);

    //TODO: Replace above code with code for disabling global interrupts 

    // Set the limit for delay[num] and clear the count for delay[num]
    limit[num] = time;
    delay_count[num] = 0;

    //Enable alarm interrupts
    timerAlarmEnable(signal_timer);

}

unsigned int delay_is_done(unsigned int num) {
    unsigned int result = 0;

    // If delay_count[num] equals limit[num], set result to 1
    if (delay_count[num] == limit[num]) {
        result = 1;
    }

    // Return the result
    return result;
}


