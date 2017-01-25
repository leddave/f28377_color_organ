#ifndef GLOBALS_H
#define GLOBALS_H

#include "F28x_Project.h"     // Device Headerfile and Examples Include File



//One of the following WS28xx #defines must be enabled! (but not both)
#define WS2811
//#define WS2812


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
#ifdef TEST_ARRAY
#define MAX_LEDS                       (56 * 4)
#else
#define MAX_LEDS                       (120 * 4)
#endif

/* In this application, strings 1,2 are left channel, 3,4 are right channel.
 * Do not exceed 4 strings, as the LED driver only drives 4 strings. */
#define NUM_STRINGS                    4
#define LED_STRING_LEN                (MAX_LEDS / NUM_STRINGS)
#define LEFT                           0
#define RIGHT                          1
#define COLOR_CHANNELS                 10
#define PEAK_FRAMES                    40

/* Define the frame states for control */
typedef enum
{
  FRAME_WAITING_FOR_START            = 0,
  FRAME_ADC_SAMPLING                 = 1,
  FRAME_RUNNING_FFT                  = 2,
  FRAME_PREP_DISPLAY_DATA            = 3,
  FRAME_SENDING_LED_DATA             = 4
} frameState;


/* Items needed for ADC and FFT. */
#define FFT_SIZE                       512
#define NUM_BINS                      (FFT_SIZE / 2)
#define SAMPLE_RATE                    44000
#define MAX_FREQ                      (SAMPLE_RATE / 2)
#define FREQ_SPACING                  (MAX_FREQ / NUM_BINS)
#define SAMPLE_CLK_FUDGE               1
#define CLOCKS_PER_SAMPLE            ((CPU_CYCLES_PER_SEC / SAMPLE_RATE) + SAMPLE_CLK_FUDGE)


/* Define Max White balanced values */
#ifdef WS2812
#define MAX_WHITE_R                    0xff
#define MAX_WHITE_G                    0x9e
#define MAX_WHITE_B                    0x68
#else
#define MAX_WHITE_R                    0xff
#define MAX_WHITE_G                    0x89
#define MAX_WHITE_B                    0x88
#endif

#define BAL_WHITE  (uint32_t)(((uint32_t)MAX_WHITE_R<<16)+((uint32_t)MAX_WHITE_G<<8)+((uint32_t)MAX_WHITE_B))

/* Define default indexes to RGB color values. */
#ifndef WS2812
#define R                              0
#define G                              1
#define B                              2

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
} Led;

#else //For WS2812(b) LEDs, redefine the color order
#define G                              0
#define R                              1
#define B                              2

//Storage array for the LED data. DO NOT change the order of these
//elements.  The fields in this struct can be used however the display
//routine needs. Only the r, g, b fields are used by the LED driver.
typedef struct
{
  uint16_t   b:8;
  uint16_t   r:8;
  uint16_t   g:8;
  uint16_t   count:7;
  uint16_t   flag:1;
} Led;
#endif

#endif
