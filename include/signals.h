//This is the header file to the .cpp file containing the internal FSM of the LED

void signals_start();  

void signals_update();

void signals_set_state(int state);

void signals_set_mode(int mode);

int signals_get_state();

void signals_off();
void signals_red(int val);
void signals_green_forward(int val);
void signals_green_left(int val);
void signals_green_right(int val);