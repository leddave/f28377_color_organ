#ifndef GLOBALS_H
#define GLOBALS_H

#include "F28x_Project.h"     // Device Headerfile and Examples Include File


/* The CPU Speed in MHz per second. Since 1 Mhz clocks means each clock is
 * equal to 1us (1 microsecond), a speed of 200 MHz means there are 200 clocks
 * in 1us. The timer routines require a period in microseconds. */
#define CPU_MHZ                        200 //F28377

#define CPU_CYCLES_PER_SEC            (CPU_MHZ * 1000000)

/* The number of times per second (frames) that LEDs are allowed to change color. */
#define FRAMES_PER_SEC                 30
#define CLOCKS_PER_FRAME              (CPU_CYCLES_PER_SEC / FRAMES_PER_SEC)

/* Define the maximum allowed (total) RGB LEDs. It should be a multiple of the
 * number of LED strings. */
#define LED_STRING_LEN                288

#define MAX_DISPLAYS                  8
#define MAX_DISP_FLAG                (1 << (MAX_DISPLAYS-1))

/* In this application, strings 1,2 are left channel, 3,4 are right channel.
 * Do not exceed 4 strings, as the LED driver only drives 4 strings. */
#define NUM_STRINGS                    4
#define MAX_LEDS                      (LED_STRING_LEN * NUM_STRINGS)

#ifdef  FLOODS
#define NUM_FLOODS                     2
#define FLOOD_STRING_LEN               256
#define FLOOD_LEDS                    (FLOOD_STRING_LEN * NUM_FLOODS)
#endif

#define LEFT                           0
#define RIGHT                          1
#define MIN_CHANNELS                   3
#define MAX_CHANNELS                   10
#define PEAK_FRAMES                    40
#define MAX_FLOOD_CHAN                 2  /* top N channels to display (per flood) at one time. */


/* Items needed for ADC and FFT. */
#define FFT_SIZE                       512
#define NUM_BINS                      (FFT_SIZE / 2)
#define SAMPLE_RATE                    44100
#define MAX_FREQ                      (SAMPLE_RATE / 2)
#define FREQ_SPACING                  (MAX_FREQ / NUM_BINS)
#define SAMPLE_CLK_FUDGE               1
#define CLOCKS_PER_SAMPLE            ((CPU_CYCLES_PER_SEC / SAMPLE_RATE) + SAMPLE_CLK_FUDGE)


#define MAX_RGB_VAL                    0xf0
#define MID_RGB_VAL                    0x78
#define MIN_RGB_VAL                    0x28

/* Define Max White balanced values */
#define MAX_WHITE_R_2812               0xff
#define MAX_WHITE_G_2812               0x9e
#define MAX_WHITE_B_2812               0x68

#define MAX_WHITE_R_2811               0xff
#define MAX_WHITE_G_2811               0x89
#define MAX_WHITE_B_2811               0x88

#define BAL_WHITE_2812  (uint32_t)(((uint32_t)MAX_WHITE_R_2812<<16)+((uint32_t)MAX_WHITE_G_2812<<8)+((uint32_t)MAX_WHITE_B_2812))
#define BAL_WHITE_2811  (uint32_t)(((uint32_t)MAX_WHITE_R_2811<<16)+((uint32_t)MAX_WHITE_G_2811<<8)+((uint32_t)MAX_WHITE_B_2811))

/* Define default indexes to RGB color values. */

//Storage array for the LED data. DO NOT change the order of these
//elements.  The fields in this struct can be used however the display
//routine needs. Only the r, g, b fields are used by the LED driver.
typedef struct
{
  uint16_t   b:8;
  uint16_t   g:8;
  uint16_t   r:8;
  uint16_t   count:7;
  uint16_t   flag:1;
} Led_2811;

typedef struct
{
  uint16_t   b:8;
  uint16_t   r:8;
  uint16_t   g:8;
  uint16_t   count:7;
  uint16_t   flag:1;
} Led_2812;

//Define the type of LEDs used for the main display and for possible floods,
//for this application. This is needed since the RGB order is not the same,
//which means that the display routines need to write data in the correct RGB
//order so that it doesn't have to be moved for bit-banging.

#define WS2811                         0
#define WS2812                         1
#define LED_MAIN                       Led_2812
#define LED_MAIN_TYPE                  WS2812
#define LED_FLOOD                      Led_2812
#define LED_FLOOD_TYPE                 WS2812

#endif
