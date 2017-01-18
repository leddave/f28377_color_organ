//###########################################################################
//
// This is my version of F2837x_PieVect.c.  I made my own because I don't
// want or need actual ISRs for every single vector - I only need one for
// Timer1. Everything else can vector to an error handling ISR.
//
//###########################################################################
#include "F2837xS_device.h"                            // F2837xS Header File Include File
//#include "F2837xS_Examples.h"                          // F2837xS Examples Include File

#define PIE_RESERVED       0
#define PIE_RESERVED_ISR   rsvd_ISR

extern __interrupt void timer1_isr(void);
__interrupt void rsvd_ISR(void);

Uint32 int_err_cnt;

const struct PIE_VECT_TABLE PieVectTableInit = {
    PIE_RESERVED_ISR,                                 // Reserved
    PIE_RESERVED_ISR,                                 // Reserved
    PIE_RESERVED_ISR,                                 // Reserved
    PIE_RESERVED_ISR,                                 // Reserved
    PIE_RESERVED_ISR,                                 // Reserved
    PIE_RESERVED_ISR,                                 // Reserved
    PIE_RESERVED_ISR,                                 // Reserved
    PIE_RESERVED_ISR,                                 // Reserved
    PIE_RESERVED_ISR,                                 // Reserved
    PIE_RESERVED_ISR,                                 // Reserved
    PIE_RESERVED_ISR,                                 // Reserved
    PIE_RESERVED_ISR,                                 // Reserved
    PIE_RESERVED_ISR,                                 // Reserved
    timer1_isr,                                       // CPU Timer 1 Interrupt
    rsvd_ISR,                                       // CPU Timer 2 Interrupt
    rsvd_ISR,                                      // Datalogging Interrupt
    rsvd_ISR,                                         // RTOS Interrupt
    rsvd_ISR,                                          // Emulation Interrupt
    rsvd_ISR,                                          // Non-Maskable Interrupt
    rsvd_ISR,                                      // Illegal Operation Trap
    rsvd_ISR,                                        // User Defined Trap 1
    rsvd_ISR,                                        // User Defined Trap 2
    rsvd_ISR,                                        // User Defined Trap 3
    rsvd_ISR,                                        // User Defined Trap 4
    rsvd_ISR,                                        // User Defined Trap 5
    rsvd_ISR,                                        // User Defined Trap 6
    rsvd_ISR,                                        // User Defined Trap 7
    rsvd_ISR,                                        // User Defined Trap 8
    rsvd_ISR,                                        // User Defined Trap 9
    rsvd_ISR,                                       // User Defined Trap 10
    rsvd_ISR,                                       // User Defined Trap 11
    rsvd_ISR,                                       // User Defined Trap 12
    rsvd_ISR,                                        // 1.1 - ADCA Interrupt 1
    rsvd_ISR,                                        // 1.2 - ADCB Interrupt 1
    rsvd_ISR,                                        // 1.3 - ADCC Interrupt 1
    rsvd_ISR,                                        // 1.4 - XINT1 Interrupt
    rsvd_ISR,                                        // 1.5 - XINT2 Interrupt
    rsvd_ISR,                                        // 1.6 - ADCD Interrupt 1
    rsvd_ISR,                                       // 1.7 - Timer 0 Interrupt
    rsvd_ISR,                                         // 1.8 - Standby and Halt Wakeup Interrupt
    rsvd_ISR,                                     // 2.1 - ePWM1 Trip Zone Interrupt
    rsvd_ISR,                                     // 2.2 - ePWM2 Trip Zone Interrupt
    rsvd_ISR,                                     // 2.3 - ePWM3 Trip Zone Interrupt
    rsvd_ISR,                                     // 2.4 - ePWM4 Trip Zone Interrupt
    rsvd_ISR,                                     // 2.5 - ePWM5 Trip Zone Interrupt
    rsvd_ISR,                                     // 2.6 - ePWM6 Trip Zone Interrupt
    rsvd_ISR,                                     // 2.7 - ePWM7 Trip Zone Interrupt
    rsvd_ISR,                                     // 2.8 - ePWM8 Trip Zone Interrupt
    rsvd_ISR,                                        // 3.1 - ePWM1 Interrupt
    rsvd_ISR,                                        // 3.2 - ePWM2 Interrupt
    rsvd_ISR,                                        // 3.3 - ePWM3 Interrupt
    rsvd_ISR,                                        // 3.4 - ePWM4 Interrupt
    rsvd_ISR,                                        // 3.5 - ePWM5 Interrupt
    rsvd_ISR,                                        // 3.6 - ePWM6 Interrupt
    rsvd_ISR,                                        // 3.7 - ePWM7 Interrupt
    rsvd_ISR,                                        // 3.8 - ePWM8 Interrupt
    rsvd_ISR,                                        // 4.1 - eCAP1 Interrupt
    rsvd_ISR,                                        // 4.2 - eCAP2 Interrupt
    rsvd_ISR,                                        // 4.3 - eCAP3 Interrupt
    rsvd_ISR,                                        // 4.4 - eCAP4 Interrupt
    rsvd_ISR,                                        // 4.5 - eCAP5 Interrupt
    rsvd_ISR,                                        // 4.6 - eCAP6 Interrupt
    rsvd_ISR,                                 // 4.7 - Reserved
    rsvd_ISR,                                 // 4.8 - Reserved
    rsvd_ISR,                                        // 5.1 - eQEP1 Interrupt
    rsvd_ISR,                                        // 5.2 - eQEP2 Interrupt
    rsvd_ISR,                                        // 5.3 - eQEP3 Interrupt
    PIE_RESERVED_ISR,                                 // 5.4 - Reserved
    PIE_RESERVED_ISR,                                 // 5.5 - Reserved
    PIE_RESERVED_ISR,                                 // 5.6 - Reserved
    PIE_RESERVED_ISR,                                 // 5.7 - Reserved
    PIE_RESERVED_ISR,                                 // 5.8 - Reserved
    rsvd_ISR,                                      // 6.1 - SPIA Receive Interrupt
    rsvd_ISR,                                      // 6.2 - SPIA Transmit Interrupt
    rsvd_ISR,                                      // 6.3 - SPIB Receive Interrupt
    rsvd_ISR,                                      // 6.4 - SPIB Transmit Interrupt
    rsvd_ISR,                                    // 6.5 - McBSPA Receive Interrupt
    rsvd_ISR,                                    // 6.6 - McBSPA Transmit Interrupt
    rsvd_ISR,                                    // 6.7 - McBSPB Receive Interrupt
    rsvd_ISR,                                    // 6.8 - McBSPB Transmit Interrupt
    rsvd_ISR,                                      // 7.1 - DMA Channel 1 Interrupt
    rsvd_ISR,                                      // 7.2 - DMA Channel 2 Interrupt
    rsvd_ISR,                                      // 7.3 - DMA Channel 3 Interrupt
    rsvd_ISR,                                      // 7.4 - DMA Channel 4 Interrupt
    rsvd_ISR,                                      // 7.5 - DMA Channel 5 Interrupt
    rsvd_ISR,                                      // 7.6 - DMA Channel 6 Interrupt
    PIE_RESERVED_ISR,                                 // 7.7 - Reserved
    PIE_RESERVED_ISR,                                 // 7.8 - Reserved
    rsvd_ISR,                                         // 8.1 - I2CA Interrupt 1
    rsvd_ISR,                                    // 8.2 - I2CA Interrupt 2
    rsvd_ISR,                                         // 8.3 - I2CB Interrupt 1
    rsvd_ISR,                                    // 8.4 - I2CB Interrupt 2
    rsvd_ISR,                                      // 8.5 - SCIC Receive Interrupt
    rsvd_ISR,                                      // 8.6 - SCIC Transmit Interrupt
    rsvd_ISR,                                      // 8.7 - SCID Receive Interrupt
    rsvd_ISR,                                      // 8.8 - SCID Transmit Interrupt
    rsvd_ISR,                                      // 9.1 - SCIA Receive Interrupt
    rsvd_ISR,                                      // 9.2 - SCIA Transmit Interrupt
    rsvd_ISR,                                      // 9.3 - SCIB Receive Interrupt
    rsvd_ISR,                                      // 9.4 - SCIB Transmit Interrupt
    rsvd_ISR,                                        // 9.5 - CANA Interrupt 0
    rsvd_ISR,                                        // 9.6 - CANA Interrupt 1
    rsvd_ISR,                                        // 9.7 - CANB Interrupt 0
    rsvd_ISR,                                        // 9.8 - CANB Interrupt 1
    rsvd_ISR,                                     // 10.1 - ADCA Event Interrupt
    rsvd_ISR,                                        // 10.2 - ADCA Interrupt 2
    rsvd_ISR,                                        // 10.3 - ADCA Interrupt 3
    rsvd_ISR,                                        // 10.4 - ADCA Interrupt 4
    rsvd_ISR,                                     // 10.5 - ADCB Event Interrupt
    rsvd_ISR,                                        // 10.6 - ADCB Interrupt 2
    rsvd_ISR,                                        // 10.7 - ADCB Interrupt 3
    rsvd_ISR,                                        // 10.8 - ADCB Interrupt 4
    rsvd_ISR,                                       // 11.1 - CLA1 Interrupt 1
    rsvd_ISR,                                       // 11.2 - CLA1 Interrupt 2
    rsvd_ISR,                                       // 11.3 - CLA1 Interrupt 3
    rsvd_ISR,                                       // 11.4 - CLA1 Interrupt 4
    rsvd_ISR,                                       // 11.5 - CLA1 Interrupt 5
    rsvd_ISR,                                       // 11.6 - CLA1 Interrupt 6
    rsvd_ISR,                                       // 11.7 - CLA1 Interrupt 7
    rsvd_ISR,                                       // 11.8 - CLA1 Interrupt 8
    rsvd_ISR,                                        // 12.1 - XINT3 Interrupt
    rsvd_ISR,                                        // 12.2 - XINT4 Interrupt
    rsvd_ISR,                                        // 12.3 - XINT5 Interrupt
    PIE_RESERVED_ISR,                                 // 12.4 - Reserved
    PIE_RESERVED_ISR,                                 // 12.5 - Reserved
    rsvd_ISR,                                          // 12.6 - VCU Interrupt
    rsvd_ISR,                                 // 12.7 - FPU Overflow Interrupt
    rsvd_ISR,                                // 12.8 - FPU Underflow Interrupt
    PIE_RESERVED_ISR,                                 // 1.9 - Reserved
    PIE_RESERVED_ISR,                                 // 1.10 - Reserved
    PIE_RESERVED_ISR,                                 // 1.11 - Reserved
    PIE_RESERVED_ISR,                                 // 1.12 - Reserved
    rsvd_ISR,                                         // 1.13 - IPC Interrupt 0
    rsvd_ISR,                                         // 1.14 - IPC Interrupt 1
    rsvd_ISR,                                         // 1.15 - IPC Interrupt 2
    rsvd_ISR,                                         // 1.16 - IPC Interrupt 3
    rsvd_ISR,                                     // 2.9 - ePWM9 Trip Zone Interrupt
    rsvd_ISR,                                    // 2.10 - ePWM10 Trip Zone Interrupt
    rsvd_ISR,                                    // 2.11 - ePWM11 Trip Zone Interrupt
    rsvd_ISR,                                    // 2.12 - ePWM12 Trip Zone Interrupt
    PIE_RESERVED_ISR,                                 // 2.13 - Reserved
    PIE_RESERVED_ISR,                                 // 2.14 - Reserved
    PIE_RESERVED_ISR,                                 // 2.15 - Reserved
    PIE_RESERVED_ISR,                                 // 2.16 - Reserved
    rsvd_ISR,                                        // 3.9 - ePWM9 Interrupt
    rsvd_ISR,                                       // 3.10 - ePWM10 Interrupt
    rsvd_ISR,                                       // 3.11 - ePWM11 Interrupt
    rsvd_ISR,                                       // 3.12 - ePWM12 Interrupt
    PIE_RESERVED_ISR,                                 // 3.13 - Reserved
    PIE_RESERVED_ISR,                                 // 3.14 - Reserved
    PIE_RESERVED_ISR,                                 // 3.15 - Reserved
    PIE_RESERVED_ISR,                                 // 3.16 - Reserved
    PIE_RESERVED_ISR,                                 // 4.9 - Reserved
    PIE_RESERVED_ISR,                                 // 4.10 - Reserved
    PIE_RESERVED_ISR,                                 // 4.11 - Reserved
    PIE_RESERVED_ISR,                                 // 4.12 - Reserved
    PIE_RESERVED_ISR,                                 // 4.13 - Reserved
    PIE_RESERVED_ISR,                                 // 4.14 - Reserved
    PIE_RESERVED_ISR,                                 // 4.15 - Reserved
    PIE_RESERVED_ISR,                                 // 4.16 - Reserved
    rsvd_ISR,                                          // 5.9 - SD1 Interrupt
    rsvd_ISR,                                          // 5.10 - SD2 Interrupt
    PIE_RESERVED_ISR,                                 // 5.11 - Reserved
    PIE_RESERVED_ISR,                                 // 5.12 - Reserved
    PIE_RESERVED_ISR,                                 // 5.13 - Reserved
    PIE_RESERVED_ISR,                                 // 5.14 - Reserved
    PIE_RESERVED_ISR,                                 // 5.15 - Reserved
    PIE_RESERVED_ISR,                                 // 5.16 - Reserved
    rsvd_ISR,                                      // 6.9 - SPIC Receive Interrupt
    rsvd_ISR,                                      // 6.10 - SPIC Transmit Interrupt
    PIE_RESERVED_ISR,                                 // 6.11 - Reserved
    PIE_RESERVED_ISR,                                 // 6.12 - Reserved
    PIE_RESERVED_ISR,                                 // 6.13 - Reserved
    PIE_RESERVED_ISR,                                 // 6.14 - Reserved
    PIE_RESERVED_ISR,                                 // 6.15 - Reserved
    PIE_RESERVED_ISR,                                 // 6.16 - Reserved
    PIE_RESERVED_ISR,                                 // 7.9 - Reserved
    PIE_RESERVED_ISR,                                 // 7.10 - Reserved
    PIE_RESERVED_ISR,                                 // 7.11 - Reserved
    PIE_RESERVED_ISR,                                 // 7.12 - Reserved
    PIE_RESERVED_ISR,                                 // 7.13 - Reserved
    PIE_RESERVED_ISR,                                 // 7.14 - Reserved
    PIE_RESERVED_ISR,                                 // 7.15 - Reserved
    PIE_RESERVED_ISR,                                 // 7.16 - Reserved
    PIE_RESERVED_ISR,                                 // 8.9 - Reserved
    PIE_RESERVED_ISR,                                 // 8.10 - Reserved
    PIE_RESERVED_ISR,                                 // 8.11 - Reserved
    PIE_RESERVED_ISR,                                 // 8.12 - Reserved
    PIE_RESERVED_ISR,                                 // 8.13 - Reserved
    PIE_RESERVED_ISR,                                 // 8.14 - Reserved
    rsvd_ISR,                                         // 8.15 - uPPA Interrupt
    PIE_RESERVED_ISR,                                 // 8.16 - Reserved
    PIE_RESERVED_ISR,                                 // 9.9 - Reserved
    PIE_RESERVED_ISR,                                 // 9.10 - Reserved
    PIE_RESERVED_ISR,                                 // 9.11 - Reserved
    PIE_RESERVED_ISR,                                 // 9.12 - Reserved
    PIE_RESERVED_ISR,                                 // 9.13 - Reserved
    PIE_RESERVED_ISR,                                 // 9.14 - Reserved
    rsvd_ISR,                                         // 9.15 - USBA Interrupt
    PIE_RESERVED_ISR,                                 // 9.16 - Reserved
    rsvd_ISR,                                     // 10.9 - ADCC Event Interrupt
    rsvd_ISR,                                        // 10.10 - ADCC Interrupt 2
    rsvd_ISR,                                        // 10.11 - ADCC Interrupt 3
    rsvd_ISR,                                        // 10.12 - ADCC Interrupt 4
    rsvd_ISR,                                     // 10.13 - ADCD Event Interrupt
    rsvd_ISR,                                        // 10.14 - ADCD Interrupt 2
    rsvd_ISR,                                        // 10.15 - ADCD Interrupt 3
    rsvd_ISR,                                        // 10.16 - ADCD Interrupt 4
    PIE_RESERVED_ISR,                                 // 11.9 - Reserved
    PIE_RESERVED_ISR,                                 // 11.10 - Reserved
    PIE_RESERVED_ISR,                                 // 11.11 - Reserved
    PIE_RESERVED_ISR,                                 // 11.12 - Reserved
    PIE_RESERVED_ISR,                                 // 11.13 - Reserved
    PIE_RESERVED_ISR,                                 // 11.14 - Reserved
    PIE_RESERVED_ISR,                                 // 11.15 - Reserved
    PIE_RESERVED_ISR,                                 // 11.16 - Reserved
    rsvd_ISR,                                   // 12.9 - EMIF Error Interrupt
    rsvd_ISR,                        // 12.10 - RAM Correctable Error Interrupt
    rsvd_ISR,                      // 12.11 - Flash Correctable Error Interrupt
    rsvd_ISR,                         // 12.12 - RAM Access Violation Interrupt
    rsvd_ISR,                                 // 12.13 - System PLL Slip Interrupt
    rsvd_ISR,                                 // 12.14 - Auxiliary PLL Slip Interrupt
    rsvd_ISR,                                 // 12.15 - CLA Overflow Interrupt
    rsvd_ISR                                 // 12.16 - CLA Underflow Interrupt
};


__interrupt void rsvd_ISR(void)
{
  int_err_cnt ++;
}


//---------------------------------------------------------------------------
// InitPieCtrl:
//---------------------------------------------------------------------------
// This function initializes the PIE control registers to a known state.
//
void InitPieCtrl(void)
{
    // Disable Interrupts at the CPU level:
    DINT;

    // Disable the PIE
    PieCtrlRegs.PIECTRL.bit.ENPIE = 0;

    // Clear all PIEIER registers:
    PieCtrlRegs.PIEIER1.all = 0;
    PieCtrlRegs.PIEIER2.all = 0;
    PieCtrlRegs.PIEIER3.all = 0;
    PieCtrlRegs.PIEIER4.all = 0;
    PieCtrlRegs.PIEIER5.all = 0;
    PieCtrlRegs.PIEIER6.all = 0;
    PieCtrlRegs.PIEIER7.all = 0;
    PieCtrlRegs.PIEIER8.all = 0;
    PieCtrlRegs.PIEIER9.all = 0;
    PieCtrlRegs.PIEIER10.all = 0;
    PieCtrlRegs.PIEIER11.all = 0;
    PieCtrlRegs.PIEIER12.all = 0;

    // Clear all PIEIFR registers:
    PieCtrlRegs.PIEIFR1.all = 0;
    PieCtrlRegs.PIEIFR2.all = 0;
    PieCtrlRegs.PIEIFR3.all = 0;
    PieCtrlRegs.PIEIFR4.all = 0;
    PieCtrlRegs.PIEIFR5.all = 0;
    PieCtrlRegs.PIEIFR6.all = 0;
    PieCtrlRegs.PIEIFR7.all = 0;
    PieCtrlRegs.PIEIFR8.all = 0;
    PieCtrlRegs.PIEIFR9.all = 0;
    PieCtrlRegs.PIEIFR10.all = 0;
    PieCtrlRegs.PIEIFR11.all = 0;
    PieCtrlRegs.PIEIFR12.all = 0;
}


//---------------------------------------------------------------------------
// InitPieVectTable:
//---------------------------------------------------------------------------
// This function initializes the PIE vector table to a known state.
// This function must be executed after boot time.

void InitPieVectTable(void)
{
    Uint16  i;
    Uint32  *Source  =  (void  *)  &PieVectTableInit;
    Uint32  *Dest  =  (void  *)  &PieVectTable;

// Do not write over first 3 32-bit locations (these locations are
// initialized by Boot ROM with boot variables)

    Source  =  Source  +  3;
    Dest  =  Dest  +  3;

    EALLOW;
    for(i  =  0;  i  <  221;  i++)
    {
    *Dest++  =  *Source++;
    }
    EDIS;

// Enable the PIE Vector Table
    PieCtrlRegs.PIECTRL.bit.ENPIE  =  1;
}
