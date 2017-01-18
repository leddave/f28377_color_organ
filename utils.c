/*****************************************************************************
 * This is the rgb_28075 project source.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "F28x_Project.h"     // Device Headerfile and Examples Include File
#include "globals.h"
#include "utils.h"

#if 0
//#ifdef FLASH
#include "f2802x_common/include/clk.h"
#include "f2802x_common/include/adc.h"
#include "f2802x_common/include/pwm.h"
#endif

//Global variables
uint32_t  randnum;

extern volatile struct CPUTIMER_REGS CpuTimer1Regs;


//Create a seed to initialize the random number sequence. If running from ram,
//(debugging) don't waste code space, just use a static seed.
void init_rnd(uint32_t seed)
{
#if 0
//#ifdef FLASH
  uint32_t seed, temp;
  CLK_Handle myClk;
  ADC_Handle myAdc;
  PWM_Handle myPwm;

  myClk = CLK_init((void *)CLK_BASE_ADDR, sizeof(CLK_Obj));
  myAdc = ADC_init((void *)ADC_BASE_ADDR, sizeof(ADC_Obj));
  myPwm = PWM_init((void *)PWM_ePWM1_BASE_ADDR, sizeof(PWM_Obj));

  // Initialize the ADC
  ADC_enableBandGap(myAdc);
  ADC_enableRefBuffers(myAdc);
  ADC_powerUp(myAdc);
  ADC_enable(myAdc);
  ADC_setVoltRefSrc(myAdc, ADC_VoltageRefSrc_Int);

  //Note: Channel ADCINA4 will be double sampled to workaround the ADC 1st sample issue for rev0 silicon errata
  ADC_setIntPulseGenMode(myAdc, ADC_IntPulseGenMode_Prior);               //ADCINT1 trips after AdcResults latch
  ADC_setSocChanNumber (myAdc, ADC_SocNumber_1, ADC_SocChanNumber_A4);    //set SOC1 channel select to ADCINA4
  ADC_setSocTrigSrc(myAdc, ADC_SocNumber_1, ADC_SocTrigSrc_EPWM1_ADCSOCA);    //set SOC1 start trigger on EPWM1A, due to round-robin SOC0 converts first then SOC1
  ADC_setSocSampleWindow(myAdc, ADC_SocNumber_1, ADC_SocSampleWindow_7_cycles);   //set SOC1 S/H Window to 7 ADC Clock Cycles, (6 ACQPS plus 1)

  // Enable PWM clock
  CLK_enablePwmClock(myClk, PWM_Number_1);

  // Setup PWM
  PWM_enableSocAPulse(myPwm);                                         // Enable SOC on A group
  PWM_setSocAPulseSrc(myPwm, PWM_SocPulseSrc_CounterEqualCmpAIncr);   // Select SOC from from CPMA on upcount
  PWM_setSocAPeriod(myPwm, PWM_SocPeriod_FirstEvent);                 // Generate pulse on 1st event
  PWM_setCmpA(myPwm, 0x0080);                                         // Set compare A value
  PWM_setPeriod(myPwm, 0xFFF);                                        // Set period for ePWM1
  PWM_setCounterMode(myPwm, PWM_CounterMode_Up);                      // count up and start
  CLK_enableTbClockSync(myClk);

  delay(4000);
  temp = ADC_readResult(myAdc, ADC_ResultNumber_1); //discard this one
  delay(4000);
  temp = ADC_readResult(myAdc, ADC_ResultNumber_1);
  delay(4000);
  seed = ADC_readResult(myAdc, ADC_ResultNumber_1);

  randnum = seed + ((temp & 0xff) << 8) + (temp >> 8);

  ADC_disable(myAdc);
  ADC_powerDown(myAdc);
  CLK_disablePwmClock(myClk, PWM_Number_1);
  CLK_disableTbClockSync(myClk);
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
