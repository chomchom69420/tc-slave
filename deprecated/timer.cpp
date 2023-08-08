#if 0

#include "configurations.h"
#include "delay.h"

static int red_time, green_time, yellow_time;

void timer_update(int timer_red, int timer_green){
    red_time = timer_red;
    green_time = timer_green;
}

void timer_start(int state_id){
    if(state_id==SLAVE_STATE_RED)   delay_set(0, red_time);
    else if(state_id==SLAVE_STATE_GREEN)    delay_set(0, green_time);
}

int timer_get_time(int state_id){
    if(state_id==SLAVE_STATE_RED)   return red_time;
    else if(state_id==SLAVE_STATE_GREEN)   return green_time;
}

int timer_get_time_elapsed(){
    return delay_get(0);
}

#endif