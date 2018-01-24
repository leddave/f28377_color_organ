/*****************************************************************************
 * This is the rgb_28075 project source.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "F28x_Project.h"     // Device Headerfile and Examples Include File
#include "globals.h"
#include "utils.h"
#include "my_adc.h"


//Global variables
uint32_t  randnum;

extern volatile struct CPUTIMER_REGS CpuTimer1Regs;
#if 0 //def FLASH
extern uint16_t RFFTinBuff1[FFT_SIZE];
extern uint16_t RFFTinBuff2[FFT_SIZE];
extern uint16_t fft_ready;

void start_adc_timer(void);
void stop_adc_timer(void);
#endif


//Create a seed to initialize the random number sequence. If running from ram,
//(debugging) don't waste code space, just use a static seed.
void init_rnd(uint32_t seed)
{
#if 0 //def FLASH
  uint16_t idx;
  uint32_t sum = 0;

  start_adc_timer();

  while (1)
  {
    if (fft_ready == 1)
    {
      fft_ready = 0;
      stop_adc_timer();
      break;
    }
  }

  for (idx = 0; idx < FFT_SIZE; idx ++)
    sum += (RFFTinBuff1[idx] + RFFTinBuff2[idx]);

  randnum = sum;
#else

  randnum = seed;
#endif
}


//This function returns a random integer between 0 and (max - 1)
//renamed from "rand" because of conflict with stdlib.h. :(
uint32_t rnd(uint32_t max)
{
  randnum = (randnum * 1664525) + 1013904223;

  if (max < 2)
    return(0);
  else
    return (randnum % max);
}


#ifdef SLAVE
#pragma CODE_SECTION(get_mode, "ramCode")

void get_mode(uint32_t *mode)
{
  uint32_t *gpio[1];
  uint32_t  temp;

  //Get address of GPIO B "data" registers on F28377S.
  gpio[0] = (uint32_t *)0x07f08; //data reg

  temp = *gpio[0];
 *mode = (temp >> 26) & 0x07;
}
#endif


#ifdef MASTER
#pragma CODE_SECTION(set_mode, "ramCode")

void set_mode(uint32_t mode)
{
  uint32_t *gpio[2];

  //Get address of GPIO B "set" and "clear" registers on F28377S.
  gpio[1] = (uint32_t *)0x07f0a; //set reg
  gpio[0] = (uint32_t *)0x07f0c; //clear reg

 *gpio[0] = 0x1c00 0000; //clear GPIOs 58, 59, 60
 *gpio[1] = ((mode & 0x7) << 26);
}
#endif


//This function holds the CPU for the indicated number of micro seconds (millionths
//of a second).  Since Timer1 is running at the CPU rate, the timeout is simply a
//multiple of this number. The timer is a countdown timer that resets to
//CLOCKS_PER_FRAME after reaching zero. To account for a delay spanning multiple
//timer rollovers, add up loop iteration delays until the total delay is reached.
void delay(uint32_t us)
{
  uint32_t  last_time;
  uint32_t  curr_time;
  uint32_t  cycles;
  uint32_t  tdelay;

  last_time = CpuTimer1Regs.TIM.all;
  cycles = us * CPU_MHZ;
  tdelay = 0;

  while (tdelay < cycles)
  {
    curr_time = CpuTimer1Regs.TIM.all;
    if (curr_time < last_time)
      tdelay += (last_time - curr_time);
    else
      tdelay += (CLOCKS_PER_FRAME - curr_time);

    last_time = curr_time;
  }
}


#if 0
float sqrt(const float m)
{
   int j;
   float i=0;
   float x1,x2;
   while( (i*i) <= m )
          i+=0.1f;
   x1=i;
   for(int j=0;j<10;j++)
   {
      x2=m;
      x2/=x1;
      x2+=x1;
      x2/=2;
      x1=x2;
   }
   return x2;
}


double sin(double x) //input must be 0.0 to 2*pi
{
  double val;

  //always wrap input angle to -PI..PI
  if (x < -3.14159265)
      x += 6.28318531;
  else
  if (x >  3.14159265)
      x -= 6.28318531;

#ifdef FAST_LO_PRECISION
  //compute sine
  if (x < 0)
      val = 1.27323954 * x + .405284735 * x * x;
  else
      val = 1.27323954 * x - 0.405284735 * x * x;
#endif


#ifdef SLOW_HI_PRECISION
  //compute sine
  if (x < 0)
  {
      val = 1.27323954 * x + .405284735 * x * x;

      if (val < 0)
          val = .225 * (val *-val - val) + val;
      else
          val = .225 * (val * val - val) + val;
  }
  else
  {
      val = 1.27323954 * x - 0.405284735 * x * x;

      if (val < 0)
          val = .225 * (val *-val - val) + val;
      else
          val = .225 * (val * val - val) + val;
  }
#endif

  return (val);
}


double cos(double x) //input must be 0.0 to 2*pi
{
  double val;

  //always wrap input angle to -PI..PI
  if (x < -3.14159265)
      x += 6.28318531;
  else
  if (x >  3.14159265)
      x -= 6.28318531;

#ifdef FAST_LO_PRECISION
  //compute cosine: sin(x + PI/2) = cos(x)
  x += 1.57079632;
  if (x >  3.14159265)
      x -= 6.28318531;

  if (x < 0)
      val = 1.27323954 * x + 0.405284735 * x * x
  else
      val = 1.27323954 * x - 0.405284735 * x * x;
#endif

#ifdef SLOW_HI_PRECISION
  //compute cosine: sin(x + PI/2) = cos(x)
  x += 1.57079632;
  if (x >  3.14159265)
      x -= 6.28318531;

  if (x < 0)
  {
      val = 1.27323954 * x + 0.405284735 * x * x;

      if (val < 0)
          val = .225 * (val *-val - val) + val;
      else
          val = .225 * (val * val - val) + val;
  }
  else
  {
      val = 1.27323954 * x - 0.405284735 * x * x;

      if (val < 0)
          val = .225 * (val *-val - val) + val;
      else
          val = .225 * (val * val - val) + val;
  }
#endif

  return (val);
}
#endif
