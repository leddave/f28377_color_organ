//###########################################################################
// FILE:   adc_soc_software_cpu01.c
// TITLE:  ADC software triggering for F2807x.
//
//! \addtogroup cpu01_example_list
//! <h1> ADC SOC Software Force (adc_soc_software)</h1>
//!
//! This example converts some voltages on ADCA and ADCB based on a software
//! trigger.
//!
//###########################################################################
#include "F28x_Project.h"
#include "string.h"
#include "rfft.h"
#include "my_adc.h"
#include "utils.h"


extern volatile struct ADC_REGS AdcaRegs;

extern volatile uint16_t AdcaResult[2];
extern uint16_t RFFTinBuff1[RFFT_SIZE];
extern uint16_t RFFTinBuff2[RFFT_SIZE];


//The input voltage range is 0 to 3.3V and the output digital code is in
//the range of 0 (0V) to 4095 (3.3V).
void adc_sample(uint16_t sample_num)
{
  //convert, wait for completion, and store results
  //start conversions immediately via software, ADCA
  //SOC0 and SOC1 will be left and right channels, depending on which
  //Launchpad pins they are connected to.
  AdcaRegs.ADCSOCFRC1.all = 0x0003; //SOC0 and SOC1

  //wait for ADCA to complete, then acknowledge flag
  while(AdcaRegs.ADCINTFLG.bit.ADCINT1 == 0);
  AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

  //store the results:
  //store the value from Launchpad pin J3-27 (left channel)
  RFFTinBuff1[sample_num] = AdcaResult[0] << 3;

  //store the value from Launchpad pin J3-29 (right channel)
//  RFFTin1Buff[sample_num+RFFT_SIZE] = AdcaResult[1] << 4;
  RFFTinBuff2[sample_num] = AdcaResult[1] << 3;
}


//The following code is taken from:
//C:\ti\controlSUITE\device_support\F2837xS\v210\F2837xS_examples_Cpu1\adc_soc_software\cpu01


//Write ADC configurations and power up ADC A
void ConfigureADC(void)
{
  EALLOW;

  //
  //write configurations
  //
  AdcaRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
  AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);

  //
  //Set pulse positions to late
  //
  AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;

  //
  //power up the ADC
  //
  AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;

  //delay for 1ms to allow ADC time to power up
  delay(1000);

  EDIS;
}


void SetupADCSoftware(void)
{
  uint16_t acqps;

  //Force 12-bit resolution (there are clock issues with 16-bit)
  acqps = 14; //75ns

  //Select the channels to convert and end of conversion flag
  EALLOW;

  //ADCA
  AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;     //SOC0 will convert pin A0
  AdcaRegs.ADCSOC0CTL.bit.ACQPS = acqps; //sample window is acqps +
                                         //1 SYSCLK cycles
  AdcaRegs.ADCSOC1CTL.bit.CHSEL = 1;     //SOC1 will convert pin A1
  AdcaRegs.ADCSOC1CTL.bit.ACQPS = acqps; //sample window is acqps +
                                         //1 SYSCLK cycles
  AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 1; //end of SOC1 will set INT1 flag
  AdcaRegs.ADCINTSEL1N2.bit.INT1E   = 1; //enable INT1 flag
  AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //make sure INT1 flag is cleared

  EDIS;
}


void adc_init(void)
{

  //Configure ADC A and power it up
  ConfigureADC();

  //Setup the ADC for software conversions
  SetupADCSoftware();
}
