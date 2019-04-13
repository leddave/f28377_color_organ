/*****************************************************************************
 * This is the f28377_color_organ project source.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "F28x_Project.h"     // Device Headerfile and Examples Include File
#include "globals.h"
#include "display.h"
#include "my_adc.h"
#include "utils.h"
#include "fpu_rfft.h"
#include "rfft.h"
#include "beat.h"
#include "msg.h"
#include "uart.h"
#ifdef USE_CLA
#include "cla.h"
#else
#include "led_driver.h"
#endif

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

#ifdef USE_CLA
#pragma DATA_SECTION(Cla1Regs,".ClaRegs");
volatile struct CLA_REGS Cla1Regs;

#pragma DATA_SECTION(MemCfgRegs,".MemCfgRegs");
volatile struct MEM_CFG_REGS MemCfgRegs;

#ifdef CLA_TIMING
#pragma DATA_SECTION(Epwm1Regs,".Epwm1Reg");
volatile struct EPWM_REGS Epwm1Regs;
#endif
#endif

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

#pragma DATA_SECTION(SciaRegs,".SciaCfg");
volatile struct SCI_REGS SciaRegs;
#pragma DATA_SECTION(ScibRegs,".ScibCfg");
volatile struct SCI_REGS ScibRegs;

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

extern uint16_t   color_channels;
extern uint16_t   last_chan_cnt;

#pragma DATA_ALIGN(chan_max_left, 4);
#pragma DATA_ALIGN(chan_max_right, 4);
#pragma DATA_SECTION(chan_max_left,"data");
#pragma DATA_SECTION(chan_max_right,"data");
uint16_t chan_max_left[MAX_CHANNELS];
uint16_t chan_max_right[MAX_CHANNELS];

//These globals are defined in the CPU -> CLA message RAM
#pragma DATA_SECTION(ppong_fill, "CpuToCla1MsgRAM")
#pragma DATA_SECTION(ppong_use, "CpuToCla1MsgRAM")
uint16_t  ppong_fill;
uint16_t  ppong_use;

volatile uint16_t frame_sync;
uint16_t   frame_start;
uint16_t   sample_count;
uint16_t   fft_ready;
uint16_t   text_showing;
uint32_t   frame_cnt;

__cregister volatile unsigned int IFR;
__cregister volatile unsigned int IER;

volatile extern uint16_t end_of_gap;
volatile extern uint16_t gap_long_found;
volatile extern uint16_t peak_flag;

#ifdef FLOODS
extern uint16_t top_chan_l[MAX_FLOOD_CHAN];
extern uint16_t top_chan_r[MAX_FLOOD_CHAN];
extern uint16_t hyst_sum_l[MAX_CHANNELS];
extern uint16_t hyst_sum_r[MAX_CHANNELS];
#endif

#ifdef CLA_TIMING
extern uint32_t  cla_tim[12];
#endif

char rx_msg[MSG_MAX_IN_MSG_LEN];


Msg_Default_Values defaults =
{
    //header
   {MSG_DEFAULT_VALUES,
    0},

    //msg body
    7,                  // number of channels
    FRAMES_PER_SEC,     // frames per second
    0x00, 0xfb,         // bitmap of available displays that are enabled (MSB, LSB)
  {{0, 0xf0, 0x00, 0x00,  0, 1},
   {1, 0x68, 0x00, 0x88,  2, 3},
   {2, 0x00, 0x00, 0xf0,  4, 7},
   {3, 0x00, 0xe0, 0x33,  8, 14},
   {4, 0x00, 0xf0, 0x00, 15, 26},
   {5, 0x60, 0xe0, 0x00, 27, 48},
   {6, 0xf0, 0xf0, 0x00, 49, 174},
   {0, 0x00, 0x00, 0x00,  0, 0},
   {0, 0x00, 0x00, 0x00,  0, 0},
   {0, 0x00, 0x00, 0x00,  0, 0}}
};


Msg_Available_Capabilities capabilities =
{
    //header
   {MSG_AVAILABLE_CAPABILITIES,
    0},

    //msg body
    16, 36,             // LED array size, each side
    16, 16,             // LED flood size
    MIN_CHANNELS,       // minimum allowable number of channels
    MAX_CHANNELS,       // maximum allowable number of channels
    FREQ_SPACING,       // resolution or spacing (in Hz) of frequency bins
    10,                 // min frames per second (update rate)
    FRAMES_PER_SEC,     // max frames per second (update rate)
    MAX_RGB_VAL,        // max red component value
    MAX_RGB_VAL,        // max green component value
    MAX_RGB_VAL,        // max blue component value
   (MAX_FREQ >> 8),     // max audio freq used (< Nyquist Frequency) - MSB
   (MAX_FREQ & 0xff),   // LSB
    MAX_DISPLAYS,       // number of available displays in Msg_Display

  {{0x00, 0x01, "Horizontal Lines"},
   {0x00, 0x02, "Vertical Lines"},
   {0x00, 0x04, "Ripples"},
   {0x00, 0x08, "Freq Equalizer"},
   {0x00, 0x10, "Chan Equalizer"},
   {0x00, 0x20, "Block Arrows"},
   {0x00, 0x40, "Block Gradient"},
   {0x00, 0x80, "Block Random"}}
};

//uint32_t isr_err = 0;
#pragma CODE_SECTION(timer1_isr, "ramCode")

//This Timer1 ISR is programmed to expire at the frame rate.
__interrupt void timer1_isr(void)
{
//if (frame_sync == 1) //count processing overruns
//isr_err ++;

  frame_start = 1; //used by the main loop to start frame processing
  frame_sync = 1;  //used by other display routines to throttle updates
  frame_cnt ++;
}


#pragma CODE_SECTION(timer2_isr, "ramCode")

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


void chip_init(void)
{
  uint16_t idx, mux;

  // Initialize System Control:
  InitSysCtrl();

  // Initialize GPIO:
  InitGpio();

  #ifdef USE_CLA
  mux = GPIO_MUX_CPU1CLA;
  #else
  mux = GPIO_MUX_CPU1;
  #endif

  #ifdef F28379
    //Setup GPIOs 0 to 3 for 4 LED string output signals
    for (idx = 0; idx < 4; idx ++)
  #else
    //Setup GPIOs 12 to 15 for 4 LED string output signals
    for (idx = 12; idx < 16; idx ++)
  #endif
    {
      GPIO_SetupPinMux(idx, mux, 0);
      GPIO_SetupPinOptions(idx, GPIO_OUTPUT, GPIO_PUSHPULL);
    }

  //GPIOs 58, 59, and 60 are used if syncing two color organs together
  #ifdef MASTER
  for (idx = 58; idx < 61; idx ++)
  {
    GPIO_SetupPinMux(idx, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(idx, GPIO_OUTPUT, GPIO_PUSHPULL);
  }
  #endif
  #ifdef SLAVE
  for (idx = 58; idx < 61; idx ++)
  {
    GPIO_SetupPinMux(idx, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(idx, GPIO_INPUT, GPIO_PUSHPULL);
  }
  #endif

  #ifdef FLOODS
  #ifdef F28379
    //Setup GPIOs 4 and 5 for left/right channel flood LEDs
    for (idx = 4; idx < 6; idx ++) //J4 pin 36 and 35
  #else
    //Setup GPIOs 16 and 17 for left/right channel flood LEDs
    for (idx = 16; idx < 18; idx ++) //J4 pin 36 and 35
  #endif
    {
      GPIO_SetupPinMux(idx, mux, 0);
      GPIO_SetupPinOptions(idx, GPIO_OUTPUT, GPIO_PUSHPULL);
    }
  #endif

    //Setup the Pinmux for SCI-A (datasheet table 4-4)
//    GPIO_SetupPinMux(28, GPIO_MUX_CPU1, 1);
//    GPIO_SetupPinOptions(28, GPIO_INPUT, GPIO_PUSHPULL);
//    GPIO_SetupPinMux(29, GPIO_MUX_CPU1, 1);
//    GPIO_SetupPinOptions(29, GPIO_OUTPUT, GPIO_ASYNC);

  //Setup the Pinmux for SCI-B (datasheet table 4-4)
  GPIO_SetupPinMux(19, GPIO_MUX_CPU1, 2); //SCIRXDB
  GPIO_SetupPinOptions(19, GPIO_INPUT, GPIO_PUSHPULL);
  GPIO_SetupPinMux(18, GPIO_MUX_CPU1, 2); //SCITXDB
  GPIO_SetupPinOptions(18, GPIO_OUTPUT, GPIO_ASYNC);
  GPIO_SetupPinMux(11, GPIO_MUX_CPU1, 0); //For BT STATE pin
  GPIO_SetupPinOptions(11, GPIO_INPUT, GPIO_PUSHPULL);

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
//ConfigCpuTimer(&CpuTimer1, 1, CLOCKS_PER_FRAME*2);
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


uint16_t connected;
uint16_t con_state;

//This function sends an AT command to the SH-HC-08 Bluetooth transceiver to
//set it up for our usage.
uint16_t send_at_cmd(char *cmd)
{
  uint16_t rx, len;

  len = strlen(cmd);
  strncpy(rx_msg, cmd, len);
  rx_msg[len]   = 13; //add <CR>
  rx_msg[len+1] = 10; //add <LF>
  rx_msg[len+2] = 0;  //null terminate

  uart_send_msg(rx_msg);
  rx = uart_recv_msg(rx_msg, 100); //wait 100ms for response

  if ((rx > 2) &&
      (rx_msg[0] == 'O') && //The transceiver is responding
      (rx_msg[1] == 'K'))
    return(1);
  else
    return(0);
}

uint16_t hist[100];
uint16_t hidx = 0;

void setup_bluetooth(void)
{
  uint16_t result;
  uint16_t f28_baud;
  uint16_t attempts;

  con_state = 0;
  attempts  = 0;
  f28_baud  = 38400;

  //First, get both devices aligned on baud rate. We assume 38400 at first.

  while (con_state != 3)
  {
    //The BT transceiver stays in AT mode until connected to a phone.
    result = send_at_cmd("AT+BAUD?"); //is baud 9600 or 38400?

    if (result) //if we got OK back, we must be at the same baud rate
    {
      con_state |= 1;
if (hidx < 100)
hist[hidx++] = 1;

      if (f28_baud == 38400)
      {
if (hidx < 100)
hist[hidx++] = 2;
        con_state |= 3; //we're done
        break;
      }
      else //currently at 9600
      {
if (hidx < 100)
hist[hidx++] = 3;
        result = send_at_cmd("AT+BAUD6");

if (result) //see if it sends a response before changing bauds
{
if (hidx < 100)
hist[hidx++] = 4;
}
        f28_baud = 38400;
        uart_init(38400); //set it to 38400
        uart_init_fifo();
        delay(100);
      }
    }
    else //no response means wrong baud or no bluetooth board is attached.
    {
      if (f28_baud == 38400)
      {
if (hidx < 100)
hist[hidx++] = 5;
        f28_baud = 9600;
        uart_init(9600); //set it to 9600
      }
      else
      {
if (hidx < 100)
hist[hidx++] = 6;
        f28_baud = 38400;
        uart_init(38400); //set it to 38400
      }

      uart_init_fifo();
      delay(100);
    }

    attempts ++;
    if (attempts == 8)
{
if (hidx < 100)
hist[hidx++] = 0xff;
      return; //give up
}
  }

  // Next set the bluetooth discoverable name to F2837x:
  attempts = 0;

  while (con_state != 7)
  {
if (hidx < 100)
hist[hidx++] = 7;
    result = send_at_cmd("AT+NAMEF2837x");

    if (result)
    {
if (hidx < 100)
hist[hidx++] = 8;
      if (strncmp(rx_msg, "OK+Name", 7) == 0)
      {
if (hidx < 100)
hist[hidx++] = 9;
        con_state |= 4; //we're done
        break;
      }
    }

    attempts ++;
    if (attempts == 4)
{
if (hidx < 100)
hist[hidx++] = 0xfe;
      return; //give up
}
  }
if (hidx < 100)
hist[hidx++] = 100;
}


void initialize(void)
{
  chip_init();
  adc_init();
  timer_init();

  uart_init(38400);
  uart_init_fifo();

  setup_bluetooth();

#if 0 // uart test
while(1)
{
  rx = uart_recv_msg(rx_msg, 10); //wait 10ms for response
  if (rx > 0)
  {
    for (idx = 0; idx < rx; idx++)
      rx_msg[idx] ++;
    uart_send_msg(rx_msg);
  }
}
#endif

#ifdef USE_CLA
  cla_init();
#else
  led_driver_init();
#endif

  display_init();
  beat_init();
  fft_init();
  init_rnd(23094);

  frame_start = 0;
  frame_sync  = 0;
  frame_cnt   = 0;
  text_showing= 0;

  //Initialize the CPU -> CLA message RAM globals
  ppong_fill  = 0;
  ppong_use   = 0;
}


int main(void)
{
  char cmd;

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
  *    [if c) or d) take too long, e) will run into the next frame.]
  */

  while (1)
  {
    if (frame_start)
    {
      start_adc_timer();
      frame_start = 0;

      get_connected_state();

      //Run beat detection in the background while samples arrive.
      beat_detect();
    }

    if (fft_ready == 1)
    {
      if (connected)
      {
        cmd = msg_recv(rx_msg);

//if (cmd != 0)
//msg_send((char *)rx_msg);
        if (cmd != 0)
          msg_process(cmd, (char *)rx_msg);
      }

      fft_ready = 0;
      stop_adc_timer();

      fft_calc();

      //Flip the ping pong flag for the display routines to fill the LED arrays
ppong_fill  = 0;
//      ppong_fill ++;
//      ppong_fill &= 0x01;


      //if the number of channels has changed, force a peak to reset the display
      if (color_channels != last_chan_cnt)
      {
        peak_flag = 1;
        last_chan_cnt = color_channels;
      }

      #ifndef FLOODS
      color_organ_prep(RFFTmagBuff1, chan_max_left);
      color_organ_prep(RFFTmagBuff2, chan_max_right);
      #else
      color_organ_prep(0, RFFTmagBuff1, chan_max_left,  top_chan_l, hyst_sum_l);
      color_organ_prep(1, RFFTmagBuff2, chan_max_right, top_chan_r, hyst_sum_r);
      #endif

      beat_detect_prep(color_channels);

      if (gap_long_found)
      {
        do_gap_display();
      }
      else
      {
        do_display(peak_flag, chan_max_left, chan_max_right);
      }

      //Update the ping pong flag for the bit-bang routines to send the data just.
      //produced. This gives the CLA the entire frame to send this data.
      ppong_use = ppong_fill;

      #ifndef USE_CLA
        led_driver();
      #else

        #ifdef CLA_TIMING
        cla_tim[8] = CpuTimer1Regs.TIM.all; //starting time
        #endif

        Cla1ForceTask1();
      #endif
    }
  }
}
