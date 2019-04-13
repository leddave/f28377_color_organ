/*****************************************************************************
 * This is the f28377_tree project source.
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "F28x_Project.h"     // Device Headerfile and Examples Include File
#include "globals.h"
#include "cla.h"


extern volatile struct EPWM_REGS Epwm1Regs;
extern volatile struct CPUTIMER_REGS CpuTimer1Regs;


#ifdef USE_CLA

#ifdef CLA_TIMING
#pragma  DATA_SECTION(cla_tim, "leddata")
uint32_t cla_tim[12];
#endif

//
// cla1Isr1 - CLA1 ISR 1
//
#pragma CODE_SECTION(cla1Isr1, "ramCode")

interrupt void cla1Isr1()
{
  #ifdef CLA_TIMING
  cla_tim[9] = CpuTimer1Regs.TIM.all; //ending time
  cla_tim[10] = cla_tim[8] - cla_tim[9];
  #endif

  PieCtrlRegs.PIEACK.all = M_INT11;
}

//
// cla1Isr1 - CLA1 ISR 2
//
#pragma CODE_SECTION(cla1Isr2, "ramCode")

interrupt void cla1Isr2()
{

  PieCtrlRegs.PIEACK.all = M_INT11;
}

//
// cla1Isr1 - CLA1 ISR 3
//
#pragma CODE_SECTION(cla1Isr3, "ramCode")

interrupt void cla1Isr3()
{
}

//
// cla1Isr1 - CLA1 ISR 4
//
#pragma CODE_SECTION(cla1Isr4, "ramCode")

interrupt void cla1Isr4()
{
}

//
// cla1Isr1 - CLA1 ISR 5
//
#pragma CODE_SECTION(cla1Isr5, "ramCode")

interrupt void cla1Isr5()
{
}

//
// cla1Isr1 - CLA1 ISR 6
//
#pragma CODE_SECTION(cla1Isr6, "ramCode")

interrupt void cla1Isr6()
{
}

//
// cla1Isr1 - CLA1 ISR 7
//
#pragma CODE_SECTION(cla1Isr7, "ramCode")

interrupt void cla1Isr7()
{
}

//
// cla1Isr1 - CLA1 ISR 8
//
#pragma CODE_SECTION(cla1Isr8, "ramCode")

interrupt void cla1Isr8()
{
  PieCtrlRegs.PIEACK.all = M_INT11;
}


#ifdef CLA_TIMING
//this function is not needed for normal use.
void init_EPWM_for_timing(void)
{
    EALLOW;
    // Assumes ePWM clock is already enabled
    Epwm1Regs.ETSEL.bit.SOCAEN    = 0;    // Disable SOC on A group
    Epwm1Regs.ETSEL.bit.SOCASEL   = 4;    // Select SOC on up-count
    Epwm1Regs.ETPS.bit.SOCAPRD    = 1;    // Generate pulse on 1st event
    Epwm1Regs.CMPA.bit.CMPA  = 0x2000;    // Set compare A value to 2048 counts
    Epwm1Regs.TBPRD          = 0x4000;    // Set period to 4096 counts
    Epwm1Regs.TBCTL.bit.CTRMODE   = 0;    // up counter

    Epwm1Regs.TBCTR = 0;
    Epwm1Regs.ETSEL.bit.SOCAEN    = 1;    // Enable SOC on A group
    EDIS;
}
#endif


//
// Cla_initMemoryMap - Initialize Memory map
//
void Cla_initMemoryMap(void)
{
    EALLOW;

    //
    // Initialize and wait for CLA1ToCPUMsgRAM
    //
    MemCfgRegs.MSGxINIT.bit.INIT_CLA1TOCPU = 1;
    while(MemCfgRegs.MSGxINITDONE.bit.INITDONE_CLA1TOCPU != 1){};

    //
    // Initialize and wait for CPUToCLA1MsgRAM
    //
    MemCfgRegs.MSGxINIT.bit.INIT_CPUTOCLA1 = 1;
    while(MemCfgRegs.MSGxINITDONE.bit.INITDONE_CPUTOCLA1 != 1){};

    //
    // Select LS5 RAM to be the programming space for the CLA
    // Select LS0/1 to be data RAM for the CLA
    //
    MemCfgRegs.LSxMSEL.bit.MSEL_LS0 = 1; //share between CPU and CLS
    MemCfgRegs.LSxMSEL.bit.MSEL_LS1 = 1;
    MemCfgRegs.LSxMSEL.bit.MSEL_LS2 = 1;
    MemCfgRegs.LSxMSEL.bit.MSEL_LS3 = 1;
    MemCfgRegs.LSxMSEL.bit.MSEL_LS4 = 1;
    MemCfgRegs.LSxMSEL.bit.MSEL_LS5 = 1;
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS0 = 0; //data mem LS0..4
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS1 = 0;
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS2 = 0;
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS3 = 0;
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS4 = 0;
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS5 = 1; //program LS5

    MemCfgRegs.LSxINIT.bit.INIT_LS0 = 1;
    while(MemCfgRegs.LSxINITDONE.bit.INITDONE_LS0 != 1){};

    MemCfgRegs.LSxINIT.bit.INIT_LS1 = 1;
    while(MemCfgRegs.LSxINITDONE.bit.INITDONE_LS1 != 1){};

    MemCfgRegs.LSxINIT.bit.INIT_LS2 = 1;
    while(MemCfgRegs.LSxINITDONE.bit.INITDONE_LS2 != 1){};

    MemCfgRegs.LSxINIT.bit.INIT_LS3 = 1;
    while(MemCfgRegs.LSxINITDONE.bit.INITDONE_LS3 != 1){};

    MemCfgRegs.LSxINIT.bit.INIT_LS4 = 1;
    while(MemCfgRegs.LSxINITDONE.bit.INITDONE_LS4 != 1){};

    EDIS;
}

//
// CLA_initCpu1Cla - Initialize CLA1 task vectors and end of task interrupts
//
void CLA_initCpu1Cla(void)
{
    //
    // Compute all CLA task vectors
    // On Type-1 CLAs the MVECT registers accept full 16-bit task addresses as
    // opposed to offsets used on older Type-0 CLAs
    //
    EALLOW;
    Cla1Regs.MVECT1 = (uint16_t)(&Cla1Task1);
//    Cla1Regs.MVECT2 = (uint16_t)(&Cla1Task2);

    //
    // Enable IACK instruction to start a task on CLA in software
    // for all  8 CLA tasks
    //
    asm("   RPT #3 || NOP");
    Cla1Regs.MCTL.bit.IACKE = 1;
    Cla1Regs.MIER.all = 0x00ff;

    //
    // Configure the vectors for the end-of-task interrupt for all
    // 8 tasks
    //
    PieVectTable.CLA1_1_INT   = &cla1Isr1;
    PieVectTable.CLA1_2_INT   = &cla1Isr2;
    PieVectTable.CLA1_3_INT   = &cla1Isr3;
    PieVectTable.CLA1_4_INT   = &cla1Isr4;
    PieVectTable.CLA1_5_INT   = &cla1Isr5;
    PieVectTable.CLA1_6_INT   = &cla1Isr6;
    PieVectTable.CLA1_7_INT   = &cla1Isr7;
    PieVectTable.CLA1_8_INT   = &cla1Isr8;

    //Set the CLA tasks to be software triggered.
//    DmaClaSrcSelRegs.CLA1TASKSRCSELx[0] = 0;

    //
    // Enable CLA interrupts at the group and subgroup levels
    //
    PieCtrlRegs.PIEIER11.all  = 0xFFFF;
    IER |= (M_INT11 );

    EINT;   // Enable Global interrupt INTM
    ERTM;   // Enable Global realtime interrupt DBGM
    EDIS;
}


void cla_init(void)
{
    EALLOW;
    CPU1_CLA1(1);                   //Enable CPU1.CLA module

    EDIS;

    Cla_initMemoryMap();

    CLA_initCpu1Cla();

    #ifdef CLA_TIMING
    init_EPWM_for_timing();
    #endif
}
#endif
