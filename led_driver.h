#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include "globals.h"

//These delays are for 200Mhz:

#ifdef WS2811

#define DELAY1     6
#define DELAY2     9
#define DELAY3a    8
#define DELAY3b    11

#else //WS2812

#define DELAY1     4
#define DELAY2     4
#define DELAY3a    0
#define DELAY3b    0

#endif

void led_driver_init(void);
void send_led_data(void);
void led_driver(void);

#endif
