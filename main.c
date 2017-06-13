/*****************************************************************************
 * This is the f28377_color_organ project source.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "F28x_Project.h"     // Device Headerfile and Examples Include File
#include "globals.h"
#include "display.h"
#include "led_driver.h"
#include "my_adc.h"
#include "utils.h"
#include "fpu_rfft.h"
#include "rfft.h"
#include "beat.h"

#ifdef FLASH
//the current f2837xS Flash API header:
#include "F021_F2837xS_C28x.h"
#endif

//Global variables (register overlays)
#pragma DATA_SECTION(WdRegs,".WdReg");
volatile struct WD_REGS WdRegs;

#pragma DATA_SECTION(CpuTimer0Regs,".Timer0Ctrl");
volatile struct CPUTIMER_REGS CpuTimer0Regs;
#pragma DATA_SECTION(CpuTimer1Regs,".Timer1Ctrl");
volatile struct CPUTIMER_REGS CpuTimer1Regs;
#pragma DATA_SECTION(CpuTimer2Regs,".Timer2Ctrl");
volatile struct CPUTIMER_REGS CpuTimer2Regs;
extern   struct CPUTIMER_VARS CpuTimer1;
extern   struct CPUTIMER_VARS CpuTimer2;

#pragma DATA_SECTION(GpioCtrlRegs,".GpioCtrl");
volatile struct GPIO_CTRL_REGS GpioCtrlRegs;
#pragma DATA_SECTION(GpioDataRegs,".GpioData");
volatile struct GPIO_DATA_REGS GpioDataRegs;
#pragma DATA_SECTION(DevCfgRegs,".DevCfg");
volatile struct DEV_CFG_REGS DevCfgRegs;

#pragma DATA_SECTION(PieCtrlRegs,".PieCtrl");
volatile struct PIE_CTRL_REGS PieCtrlRegs;
#pragma DATA_SECTION(PieVectTable,".PieVect");
volatile struct PIE_VECT_TABLE PieVectTable;
#pragma DATA_SECTION(InputXbarRegs,".InputXbar");
volatile struct INPUT_XBAR_REGS InputXbarRegs;
#pragma DATA_SECTION(ClkCfgRegs,".ClkCfg");
volatile struct CLK_CFG_REGS ClkCfgRegs;

#pragma DATA_SECTION(Flash0EccRegs,".Flash0Ecc");
volatile struct FLASH_ECC_REGS Flash0EccRegs;
#pragma DATA_SECTION(Flash0CtrlRegs,".Flash0Ctrl");
volatile struct FLASH_CTRL_REGS Flash0CtrlRegs;
#pragma DATA_SECTION(Flash1EccRegs,".Flash1Ecc");
volatile struct FLASH_ECC_REGS Flash1EccRegs;
#pragma DATA_SECTION(Flash1CtrlRegs,".Flash1Ctrl");
volatile struct FLASH_CTRL_REGS Flash1CtrlRegs;

#pragma DATA_SECTION(CpuSysRegs,".CpuSys");
volatile struct CPU_SYS_REGS CpuSysRegs;

#pragma DATA_SECTION(AnalogSubsysRegs,".AnalogSubsys");
volatile struct ANALOG_SUBSYS_REGS AnalogSubsysRegs;
#pragma DATA_SECTION(AdcaRegs,".AdcA");
volatile struct ADC_REGS AdcaRegs;
#pragma DATA_SECTION(AdcbRegs,".AdcB");
volatile struct ADC_REGS AdcbRegs;
#pragma DATA_SECTION(AdccRegs,".AdcC");
volatile struct ADC_REGS AdccRegs;
#pragma DATA_SECTION(AdcdRegs,".AdcD");
volatile struct ADC_REGS AdcdRegs;
#pragma DATA_SECTION(AdcaResult,".AdcaResult");
volatile uint16_t AdcaResult[2];

extern uint16_t RFFTin1Buff[2*RFFT_SIZE];
extern float RFFTmagBuff1[RFFT_SIZE/2+1];
extern float RFFTmagBuff2[RFFT_SIZE/2+1];

#pragma DATA_SECTION(chan_max_left,"data");
#pragma DATA_SECTION(chan_max_right,"data");
uint16_t chan_max_left[COLOR_CHANNELS];
uint16_t chan_max_right[COLOR_CHANNELS];

uint16_t   frame_start;
frameState frame_state;
uint16_t   sample_count;
uint16_t   fft_ready;
uint16_t   text_showing;

__cregister volatile unsigned int IFR;
__cregister volatile unsigned int IER;

extern volatile uint16_t frame_sync;
extern uint32_t frame_cnt;
volatile extern uint16_t end_of_gap;
volatile extern uint16_t gap_long_found;
volatile extern uint16_t peak_flag;


//This Timer1 ISR is programmed to expire at the frame rate.
__interrupt void timer1_isr(void)
{
  frame_start = 1; //used by the main loop to start frame processing
  frame_sync = 1;  //used by other display routines to throttle updates
  frame_cnt ++;
}


//The ISR will store each sampled value in the FFT buffer, and
//raise the flag once the buffer is full.
__interrupt void timer2_isr(void)
{
  adc_sample(sample_count++);
  if (sample_count == (RFFT_SIZE - 1))
  {
    sample_count = 0;
    fft_ready = 1;
  }

  AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
  PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}


void start_adc_timer(void)
{
  CpuTimer2Regs.TCR.bit.TRB = 1;     // 1 = reload timer
  CpuTimer2Regs.TCR.bit.TSS = 0;     // 0 = Start/Restart
}


void stop_adc_timer(void)
{
  CpuTimer2Regs.TCR.bit.TSS = 1;     // 1 = Stop timer
  CpuTimer2Regs.TCR.bit.TRB = 1;     // 1 = reload timer
}


void chip_init(void)
{
  uint16_t idx;

  // Initialize System Control:
  InitSysCtrl();

  // Initialize GPIO:
  InitGpio();

  //Setup GPIOs 12 to 15 for 4 LED string output signals
  for (idx = 12; idx < 16; idx ++)
  {
    GPIO_SetupPinMux(idx, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(idx, GPIO_OUTPUT, GPIO_PUSHPULL);
  }

  //GPIOs 16 and 17 are used if syncing two color organs together
  #ifdef MASTER
  for (idx = 16; idx < 18; idx ++)
  {
    GPIO_SetupPinMux(idx, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(idx, GPIO_OUTPUT, GPIO_PUSHPULL);
  }
  #endif
  #ifdef SLAVE
  for (idx = 16; idx < 18; idx ++)
  {
    GPIO_SetupPinMux(idx, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(idx, GPIO_INPUT, GPIO_PUSHPULL);
  }
  #endif

  // Disable CPU interrupts
  DINT;
}


// This function stops and initializes TIMER1. TIMER1 is used because it
// triggers INT13 directly.
void timer_init(void)
{
  // Initialize the PIE control registers to their default state.
  // The default state is all PIE interrupts disabled and flags
  // are cleared.
  InitPieCtrl();

  // Disable CPU interrupts and clear all CPU interrupt flags:
  IER = 0x0000;
  IFR = 0x0000;

  // Initialize the PIE vector table with pointers to the shell Interrupt
  // Service Routines (ISR).
  // This will populate the entire table, even if the interrupt
  // is not used in this example.  This is useful for debug purposes.
  // The shell ISR routines are found in F2807x_DefaultIsr.c.
  InitPieVectTable();

  // Interrupts that are used in this example are re-mapped to
  // ISR functions found within this file.
  EALLOW;  // This is needed to write to EALLOW protected registers
  PieVectTable.TIMER1_INT = &timer1_isr;
  PieVectTable.TIMER2_INT = &timer2_isr;
  EDIS;    // This is needed to disable write to EALLOW protected registers

  // Step 4. Initialize the Device Peripheral. This function can be
  //         found in F2806x_CpuTimers.c
  InitCpuTimers();   // For this example, only initialize the Cpu Timers

  ConfigCpuTimer(&CpuTimer1, 1, CLOCKS_PER_FRAME);
  ConfigCpuTimer(&CpuTimer2, 1, CLOCKS_PER_SAMPLE);

  // To ensure precise timing, use write-only instructions to write to the entire register. Therefore, if any
  // of the configuration bits are changed in ConfigCpuTimer and InitCpuTimers (in F2806x_CpuTimers.h), the
  // below settings must also be updated.

  //Start the frame timer but not the sample timer.
  CpuTimer1Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0

  // Enable CPU-Timer 1 interrupts
  // IER |= M_INT1;  //for Timer 0
  IER |= M_INT13; //for Timer 1
  IER |= M_INT14; //for Timer 2

  // Enable TINT0 in the PIE: Group 1 interrupt 7
  PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

  // Enable global Interrupts and higher priority real-time debug events:
  EINT;  // Enable Global interrupt INTM
  ERTM;  // Enable Global realtime interrupt DBGM
}


void initialize(void)
{
  chip_init();
  adc_init();
  timer_init();

  led_driver_init();
  display_init();
  beat_init();
  fft_init();
  init_rnd(23094);

  frame_state = FRAME_WAITING_FOR_START;
  frame_start = 0;
  frame_cnt   = 0;
  text_showing= 0;
}


int main(void)
{

  //The init function must be called prior to flash copies that will fail
  //if the watchdog timer expires prior to the memcpy completion. The init
  //function will disable the watchdog.

  initialize();

  initial_display();

 /* Here is the processing schedule for each frame:
  *
  * Frame Timer expires:
  *  1) Start ADC Sampling Timer
  *  2) Call Beat Detect to run in the background (since it runs multi-frame)
  *  3) At sample 512, kick off the processing:
  *     a) Stop the Sampling Timer
  *     b) Run the FFT for both channels
  *     c) Prep the FFT outputs
  *     d) Run the Display routine
  *     e) Call the LED driver to update the LED strings.
  *
  *    [if c) and d) take too long, e) will run into the next frame.]
  */

  while (1)
  {
    if (frame_start)
    {
      start_adc_timer();
      frame_start = 0;
      frame_state = FRAME_ADC_SAMPLING;

      //Run beat detection in the background while samples arrive.
      beat_detect();
    }

    if (fft_ready == 1)
    {
      fft_ready = 0;
      stop_adc_timer();

      frame_state = FRAME_RUNNING_FFT;
      fft_calc();

      frame_state = FRAME_PREP_DISPLAY_DATA;
      color_organ_prep(RFFTmagBuff1, chan_max_left);
      color_organ_prep(RFFTmagBuff2, chan_max_right);
      beat_detect_prep();

      if (gap_long_found)
      {
        do_gap_display();
      }
      else
      {
        //Create a display with the data. There could be logic here to use
        //the output of beat detection to periodically change display routines.

        //For now, fix the display on 2x2 until we figure out a good time to change.
        two_by_two(peak_flag, chan_max_left, chan_max_right);

#if 0
      if (end_of_gap)
      {
        display ++;
        if (display == 2)
          display = 0;
      }

      switch (display)
      {
        case 0:
          line_segments(peak_flag, chan_max_right);
          break;

        case 1:
          two_by_two(peak_flag, chan_max_left, chan_max_right);
          break;

        case 2:
          tetris(LEFT,  3, peak_flag, chan_max_left);
          tetris(RIGHT, 3, peak_flag, chan_max_right);
          break;
      };
#endif

        frame_state = FRAME_SENDING_LED_DATA;
        led_driver();
      }

      frame_sync = 0;
      frame_state = FRAME_WAITING_FOR_START;
    }
  }
}
