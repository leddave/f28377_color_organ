#include "string.h"
#include "fpu_rfft.h"
#include "math.h"
#include "rfft.h"


//! \brief FFT Calculation Buffer
//! \note The input buffer needs to be aligned to an 2N word boundary
//! \note If the number of FFT stages is odd, the result of the FFT will
//! be written to this buffer
//! \note this buffer takes N 12-bit ADC input, hence its defined as an
//! unsigned int array, but the FFT algorithm ping-pongs between input/
//! output buffers each stage, therefore this buffer needs to be able
//! to accommodate N floats and should be of size 2*RFFT_SIZE

//#pragma DATA_ALIGN(RFFTin1Buff,2*RFFT_SIZE)
//#pragma DATA_SECTION(RFFTin1Buff,"RFFTdata1")
//uint16_t RFFTin1Buff[2*RFFT_SIZE];
#pragma DATA_ALIGN(RFFTinBuff1,2*RFFT_SIZE)
#pragma DATA_SECTION(RFFTinBuff1,"RFFTdata1")
uint16_t RFFTinBuff1[RFFT_SIZE];
#pragma DATA_ALIGN(RFFTinBuff2,2*RFFT_SIZE)
#pragma DATA_SECTION(RFFTinBuff2,"RFFTdata1")
uint16_t RFFTinBuff2[RFFT_SIZE];

#pragma DATA_ALIGN(RFFTmagBuff1,RFFT_SIZE)
#pragma DATA_SECTION(RFFTmagBuff1,"RFFTdata2")
float RFFTmagBuff1[RFFT_SIZE/2+1];

#pragma DATA_ALIGN(RFFTmagBuff2,RFFT_SIZE)
#pragma DATA_SECTION(RFFTmagBuff2,"RFFTdata2")
float RFFTmagBuff2[RFFT_SIZE/2+1];

//! \brief FFT Calculation Buffer
//! \note If the number of FFT stages is even, the result of the FFT will
//! be written to this buffer
//!
#pragma DATA_ALIGN(RFFToutBuff,2*RFFT_SIZE)
#pragma DATA_SECTION(RFFToutBuff,"RFFTdata3")
float RFFToutBuff[RFFT_SIZE];

//! \brief Twiddle Factors
#pragma DATA_ALIGN(RFFTF32Coef,RFFT_SIZE)
#pragma DATA_SECTION(RFFTF32Coef,"RFFTdata4")
float RFFTF32Coef[RFFT_SIZE];

//! \brief RFFT_ADC_F32_STRUCT object
//!
RFFT_ADC_F32_STRUCT rfft_adc;

//! \brief Handle to the RFFT_ADC_F32_STRUCT object
//!
RFFT_ADC_F32_STRUCT_Handle hnd_rfft_adc = &rfft_adc;

//! \brief RFFT_F32_STRUCT object
//!
RFFT_F32_STRUCT rfft;

//! \brief Handle to the RFFT_F32_STRUCT object
//!
RFFT_F32_STRUCT_Handle hnd_rfft = &rfft;


extern uint16_t sample_count;
extern uint16_t fft_ready;


#pragma CODE_SECTION(edge_taper, "ramCode")

// This function does a 16 sample edge tapering of the FFT inputs for both
// left and right channels.
void edge_taper(void)
{
  uint16_t idx1, idx2;
  uint16_t idx3, idx4;
  uint16_t shift;

  shift = 15;
  idx2 = RFFT_SIZE-1;
  idx3 = RFFT_SIZE;
  idx4 =(RFFT_SIZE*2)-1;

  for (idx1 = 0; idx1 < 15; idx1 ++)
  {
    RFFTinBuff1[idx1] >>= shift; //left channel
    RFFTinBuff1[idx2] >>= shift;
//    RFFTinBuff2[idx3] >>= shift; //right channel
//    RFFTinBuff2[idx4] >>= shift;
    RFFTinBuff2[idx1] >>= shift; //right channel
    RFFTinBuff2[idx2] >>= shift;
    shift --;
    idx2 --;
    idx3 ++;
    idx4 --;
  }
}


//One time init for RFFT.
void fft_init()
{
  sample_count = 0;
  fft_ready = 0;

  hnd_rfft_adc->Tail  = &(hnd_rfft->OutBuf);

  hnd_rfft->FFTSize   = RFFT_SIZE;       //FFT size
  hnd_rfft->FFTStages = RFFT_STAGES;     //FFT stages

  hnd_rfft_adc->InBuf = &RFFTinBuff1[0]; //Input buffer (12-bit ADC) input
  hnd_rfft->OutBuf    = &RFFToutBuff[0]; //Output buffer
  hnd_rfft->CosSinBuf = &RFFTF32Coef[0]; //Twiddle factor
  hnd_rfft->MagBuf    = &RFFTmagBuff1[0]; //Magnitude output buffer

  RFFT_f32_sincostable(hnd_rfft);        //Calculate twiddle factor

  memset(RFFToutBuff, 0, sizeof(RFFToutBuff));
//  memset(RFFTmagBuff, 0, sizeof(RFFTmagBuff));
}


#pragma CODE_SECTION(fft_calc, "ramCode")

void fft_calc(void)
{
  edge_taper();

  //Process the left channel
  hnd_rfft_adc->InBuf = &RFFTinBuff1[0];  //Left channel input buffer
  RFFT_adc_f32(hnd_rfft_adc);             // Calculate real FFT with ADC input

  hnd_rfft->MagBuf    = &RFFTmagBuff1[0]; //Magnitude output buffer
  RFFT_f32_mag(hnd_rfft);                 //Calculate magnitude


  //Process the right channel
  hnd_rfft_adc->InBuf = &RFFTinBuff2[0];  //Right channel input buffer
  RFFT_adc_f32(hnd_rfft_adc);             // Calculate real FFT with ADC input

  hnd_rfft->MagBuf    = &RFFTmagBuff2[0]; //Magnitude output buffer
  RFFT_f32_mag(hnd_rfft);                 //Calculate magnitude
}
