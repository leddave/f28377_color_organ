/*****************************************************************************
 * These file contains code to drive 24bit color data to strings of WS2811
 * or WS2812b addressable LEDs. The functions contained in this file are
 * timed precisely for the given CPU_SPEED. They must be executed out of
 * on-chip RAM (running out of flash will cause them to run too slowly).
 *
 * NOTE: This file requires that one of these symbols is defined:
 *       WS2811
 *       WS2812
 *
 * These functions buffer the array of RGB values for each LED at the rate
 * specified by FRAMES_PER_SEC, and control the toggling of GPIOs to acheive
 * the desired colors. The user (incoming) array may be updated asynchronously
 * but captured at frame interval timer events.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "led_driver.h"
#include "display.h"


//Global variables
uint32_t  frame_cnt;
uint16_t  pause;

#ifdef TIMING
uint32_t  time_start;
uint32_t  time_end;
uint32_t  time_idx;
uint32_t  times[20];
#endif

volatile uint16_t frame_sync;
extern   Led      led[MAX_LEDS]; //array of colors built elsewhere

extern volatile struct CPUTIMER_REGS  CpuTimer1Regs;
extern volatile struct GPIO_DATA_REGS GpioDataRegs;


void led_driver_init(void)
{
  frame_cnt  = 0;
  frame_sync = 0;
  pause      = 0;
#ifdef TIMING
  time_idx   = 0;
  memset(times, 0, sizeof(times));
#endif
}


#pragma CODE_SECTION(send_led_data, "ramCode")

//This function bit bangs LED bit data to four equal length strings.
//It writes them to GPIOs 12 through 15, which are on F28377 Launchpad
//Header J4, pins 40 through 37 respectively.  Less than four strings
//can be used, as defined by globals.h.  It will not affect the timing
//of this function.  Since this function assumes led[] is divided into
//four equal strings, the code will stream garbage to unused GPIOs.
//
//NOTE: This code must not be interrupted, or LEDs will flash wildly.
void send_led_data(void)
{
  uint16_t  idx;
  uint16_t  idx2;
  uint16_t  bits;
  uint16_t  eow;
  uint32_t  mask_0;
  uint32_t  mask_1;
  uint32_t  clr1;
  uint32_t  clr2;
  uint32_t  clr3;
  uint32_t  clr4;
  uint32_t *str1;
  uint32_t *str2;
  uint32_t *str3;
  uint32_t *str4;
  uint32_t *gpio[2];

  DINT;

  //Get address of GPIO A "set" and "clear" registers on F28377S.
  //GPIO A Set reg:  0x07f02
  //GPIO A Clr reg:  0x07f04
  gpio[1] = (uint32_t *)0x07f02; //set reg
  gpio[0] = (uint32_t *)0x07f04; //clear reg

#ifdef TIMING
  time_start = CpuTimer1Regs.TIM.all;
#endif

  bits  = LED_STRING_LEN * 8 * 3;
  str1 = (uint32_t *)&led[0];                 //Left channel, str1 and str2
  str2 = (uint32_t *)&led[LED_STRING_LEN];
  str3 = (uint32_t *)&led[LED_STRING_LEN*2];  //Right channel, str3 and str4
  str4 = (uint32_t *)&led[LED_STRING_LEN*3];
  eow  = 0;

  //Prepare the loop. The flip32 intrinsic bit reverses a 32bit word. In our
  //24bit RGB case, this puts 8 empty bits at the bottom of the 32 bits.
  clr1 = __flip32(*str1++) >> 4;
  clr2 = __flip32(*str2++) >> 3;
  clr3 = __flip32(*str3++) >> 2;
  clr4 = __flip32(*str4++) >> 1;

  //Create a 1's mask and a 0's mask for the 4 LED strings (for GPIOs 12..15).
  mask_1 = ((clr1 & 0x10) | (clr2 & 0x20) | (clr3 & 0x40) | (clr4 & 0x80)) << 8;
  mask_0 = ~mask_1 & 0xf000;

  //Clock the data out to the lights. This code will drive 4 consecutive GPIOs
  //simultaneously, for 4 equal-length arrays of LEDs.
  for (idx = 0; idx < bits; idx ++)
  {

    //***TIME 0: Set all 4 GPIOs (12..15) high at clock 0.
   *gpio[1] = 0x0000f000;

    for (idx2 = 0; idx2 < DELAY1; idx2 ++)
      asm("  nop ");

    asm("  nop ");
    asm("  nop ");
    asm("  nop ");
#ifdef WS2812
    asm("  nop ");
    asm("  nop ");
    asm("  nop ");
    asm("  nop ");
    asm("  nop ");
    asm("  nop ");
    asm("  nop ");
    asm("  nop ");
#endif

    //***TIME 1: Set GPIOs for zero bits low.
    //(ws2811 = clock 100, ws2812 = clock 80)
   *gpio[0] = mask_0;

    for (idx2 = 0; idx2 < DELAY2; idx2 ++)
      asm("  nop ");

    asm("  nop ");
    asm("  nop ");
#ifdef WS2812
    asm("  nop ");
    asm("  nop ");
    asm("  nop ");
    asm("  nop ");
    asm("  nop ");
    asm("  nop ");
    asm("  nop ");
    asm("  nop ");
    asm("  nop ");
    asm("  nop ");
#endif

    //***TIME 2: Set GPIOs for one bits low.
    //(ws2811 = clock 240, ws2812 = clock 160)
   *gpio[0] = mask_1;


    //***TIME 3: Wait for end of bit time (prepare for the next bit).
    //(ws2811 requires 500 clocks (at 200Mhz) per bit).
    //(ws2812 requires 250 clocks (at 200Mhz) per bit).

    eow ++;
    if (eow == 24) //time to load the next led[]
    {
      eow  = 0;
      clr1 = __flip32(*str1++) >> 4;
      clr2 = __flip32(*str2++) >> 3;
      clr3 = __flip32(*str3++) >> 2;
      clr4 = __flip32(*str4++) >> 1;

#ifdef WS2811
      for (idx2 = 0; idx2 < DELAY3a; idx2 ++)
        asm("  nop ");

      asm("  nop ");
      asm("  nop ");
      asm("  nop ");
      asm("  nop ");
      asm("  nop ");
      asm("  nop ");
      asm("  nop ");
      asm("  nop ");
      asm("  nop ");
#endif
    }
    else
    {
      clr1 >>= 1;
      clr2 >>= 1;
      clr3 >>= 1;
      clr4 >>= 1;

#ifdef WS2811
      for (idx2 = 0; idx2 < DELAY3b; idx2 ++)
        asm("  nop ");

      asm("  nop ");
      asm("  nop ");
      asm("  nop ");
      asm("  nop ");
      asm("  nop ");
      asm("  nop ");
#endif
    }

    //Create a 1's mask and a 0's mask for the 4 LED strings (for GPIOs 12..15).
    mask_1 = ((clr1 & 0x10) | (clr2 & 0x20) | (clr3 & 0x40) | (clr4 & 0x80)) << 8;
    mask_0 = ~mask_1 & 0xf000;
  }


  EINT;

#ifdef TIMING
  time_end = CpuTimer1Regs.TIM.all;

  //Since Timer1 is running at the CPU clock rate, we can use it to see how
  //long it takes to run this driver.  Keep the last 10.
  times[time_idx++] = time_start - time_end;

  if (time_idx == 20) time_idx = 0;
#endif
}


// This main driver function should be called at each timer interrupt. WS8211
// and WS2812 LEDs require 24bits/LED when the colors need to change. Since
// PWM is not required, this function should called at the frame rate.
void led_driver(void)
{

  if (pause == 0)
  {
    send_led_data();
  }

  //Set frame_sync for pattern generation (1 means the data was sent)
  frame_sync = 0;
}
