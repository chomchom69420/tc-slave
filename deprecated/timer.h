#if 0

//Update the state_id (red/green/yellow) with the corresponding time
#define DEFAULT_TIMER_RED 60
#define DEFAULT_TIMER_GREEN 60

void timer_update(int timer_red, int timer_green);

//Set the delay for the corresponding time for the specific state_id
void timer_start(int state_id);

int timer_get_time(int state_id);

int timer_get_time_elapsed();

#endif