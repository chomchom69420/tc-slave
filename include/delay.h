/********************************************************
 * delay
 *
 * this file provides function declarations for Traffic Controller
 * timer-based delay library functions.
 * 
 * Copyright(C) 2023, Soham Chakaborty and Tanmoy Dutta 
 * All rights reserved
 *
 */
#ifndef DELAY_H_INCLUDED
#define DELAY_H_INCLUDED

/*initialize the timer with correct prescaler, clock frequency and overflow interrupt value, enable interrupts*/
void delay_init();

/* get the current tick counter value for the specified instance */
unsigned delay_get(unsigned num);

/* set the counter limit and reset the count for the specified instance */
void delay_set(unsigned int num, unsigned int time_in_s);

/* return 1 if the specified instance of the counter has reached its
* limit, otherwise, return 0 */
unsigned delay_is_done(unsigned int num);

#endif // DELAY_H_INCLUDED