#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include "globals.h"

//These delays are for 200Mhz:

//#ifdef WS2811

//#define DELAY1     6
//#define DELAY2     9
//#define DELAY3a    13
//#define DELAY3b    14

//#else //WS2812

#define DELAY1     6  /* 4 */
#define DELAY2     7 /* 4 */
#define DELAY3a    1  /* 1 */
#define DELAY3b    2 /* 2 */

//#endif

void led_driver_init(void);
void send_led_data(void);
void led_driver(void);

#endif
