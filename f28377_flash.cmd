// See F2807x_Headers__nonBIOS.cmd for all register areas
-x
-stack 0x400
-heap  0x100

// F28377 Memory Layout:
// Dedicated RAM (CPU Only):    0x000000 - 0x0007ff    (2K:  M0, M1)
// Message RAM (CLA -> CPU):    0x001480 - 0x0014ff    (128B)
//             (CPU -> CLA):    0x001500 - 0x00157f    (128B)
// UPP Msg RAM (Tx):            0x006c00 - 0x006dff    (512B)
//             (Rx):            0x006e00 - 0x006fff    (512B)
// LS0..5 RAM:                  0x008000 - 0x00afff    (2KB * 6)
// D0..1 RAM:                   0x00b000 - 0x00bfff    (2KB * 2)
// GS0..15 RAM:                 0x00c000 - 0x01bfff    (4KB * 16)
// CAN A RAM:                   0x049000 - 0x0497ff    (2KB)
// CAN B RAM:                   0x04b000 - 0x04b7ff    (2KB)
// Flash:                       0x080000 - 0x0fffff    (256KB * 2)
// Boot ROM:                    0x3f8000 - 0x3fffbf    (32K-64B)
// Vectors:                     0x3fffc0 - 0x3fffff    (64B)


MEMORY
{
    PAGE 0:
    PROG_MEM0   :   origin = 0x00000122  length = 0x000002DE
    CLA_RAM     :   origin = 0x00001480  length = 0x00000100
    UPP_RAM     :   origin = 0x00006c00  length = 0x00000400
    PROG_MEM1   :   origin = 0x00008000  length = 0x00010000
    BEGIN_FLASH :   origin = 0x00080000  length = 0x00000010  /* boot rom jumps to here */
    FLASH_A     :   origin = 0x00080010  length = 0x0003fff0
    CANA_RAM    :   origin = 0x00049000  length = 0x00000800
    CANB_RAM    :   origin = 0x0004b000  length = 0x00000800
    RESET       :   origin = 0x003FFFC0, length = 0x00000002

    PAGE 1:
    BOOT_RSVD   :   origin = 0x00000002  length = 0x00000120 /* Part of M0, BOOT rom will use this for stack */
    STACK       :   origin = 0x00000400  length = 0x00000400
    ADCA_RESULT :   origin = 0x00000b00  length = 0x00000020
    ADCB_RESULT :   origin = 0x00000b20  length = 0x00000020
    CPU_TIMER0  :   origin = 0x00000c00  length = 0x00000008
    CPU_TIMER1  :   origin = 0x00000c08  length = 0x00000008
    CPU_TIMER2  :   origin = 0x00000c10  length = 0x00000008
    PIE_CTRL    :   origin = 0x00000CE0  length = 0x00000020  /* PIE control registers */
    PIE_VECT    :   origin = 0x00000D00  length = 0x00000200  /* PIE Vector Table */
    WD_REGS     :   origin = 0x00007000  length = 0x00000040
    ADCA_REGS   :   origin = 0x00007400  length = 0x00000080
    ADCB_REGS   :   origin = 0x00007480  length = 0x00000080
    ADCC_REGS   :   origin = 0x00007500  length = 0x00000080
    ADCD_REGS   :   origin = 0x00007580  length = 0x00000080
    GPIO_CTRL   :   origin = 0x00007c00  length = 0x00000180
    GPIO_DATA   :   origin = 0x00007f00  length = 0x00000030
    INPUT_XBAR  :   origin = 0x00007900  length = 0x00000040
    DATA1       :   origin = 0x00018000  length = 0x00004000

    FLASH_PUMP  :   origin = 0x00050024  length = 0x00000002
    DEV_CFG     :   origin = 0x0005D000  length = 0x00000180
    ANALOG_SUBSYS : origin = 0x0005D180  length = 0x00000080
    CPU_SYS     :   origin = 0x0005D300  length = 0x00000100
    CLK_CFG     :   origin = 0x0005D200  length = 0x00000100
    FLASH0_CTRL :   origin = 0x0005F800  length = 0x00000300
    FLASH0_ECC  :   origin = 0x0005FB00  length = 0x00000040
    FLASH1_CTRL :   origin = 0x0005Fc00  length = 0x00000300
    FLASH1_ECC  :   origin = 0x0005Ff00  length = 0x00000040
    FLASH_B     :   origin = 0x000c0000  length = 0x00040000
    BOOT_ROM    :   origin = 0x003f8000  length = 0x00007fc0
}

SECTIONS
{
    /* PAGE 0 (program) */
    .text:      >   FLASH_A       PAGE=0
    .cinit:     >   FLASH_A       PAGE=0
    .pinit:     >   FLASH_A       PAGE=0
    .heap:      >   CANB_RAM      PAGE=0
    .reset:     >   RESET,        PAGE=0, TYPE = DSECT /* not used */

    /* PAGE 1 (data) */

    .stack:     >   STACK         PAGE=1
    .bss:       >   DATA1         PAGE=1
    .system:    >   DATA1         PAGE=1
    .sysmem:    >   DATA1         PAGE=1
    .AdcaResult: >  ADCA_RESULT   PAGE=1
    .AdcbResult: >  ADCB_RESULT   PAGE=1
    .Timer0Ctrl: >  CPU_TIMER0    PAGE=1
    .Timer1Ctrl: >  CPU_TIMER1    PAGE=1
    .Timer2Ctrl: >  CPU_TIMER2    PAGE=1
    .GpioCtrl:  >   GPIO_CTRL     PAGE=1
    .GpioData:  >   GPIO_DATA     PAGE=1
    .WdReg:     >   WD_REGS       PAGE=1
    .AdcA:      >   ADCA_REGS     PAGE=1
    .AdcB:      >   ADCB_REGS     PAGE=1
    .AdcC:      >   ADCC_REGS     PAGE=1
    .AdcD:      >   ADCD_REGS     PAGE=1
    .ClkCfg:    >   CLK_CFG       PAGE=1
    .DevCfg:    >   DEV_CFG       PAGE=1
    .PieCtrl:   >   PIE_CTRL      PAGE=1
    .PieVect:   >   PIE_VECT      PAGE=1
    .InputXbar: >   INPUT_XBAR    PAGE=1
    .FlashPump  >   FLASH_PUMP    PAGE=1
    .Flash0Ecc: >   FLASH0_ECC    PAGE=1
    .Flash0Ctrl: >  FLASH0_CTRL   PAGE=1
    .Flash1Ecc: >   FLASH1_ECC    PAGE=1
    .Flash1Ctrl: >  FLASH1_CTRL   PAGE=1
    .CpuSys:    >   CPU_SYS       PAGE=1
    .AnalogSubsys: >ANALOG_SUBSYS PAGE=1
    RFFTdata1   >   DATA1         PAGE=1
    RFFTdata2   >   DATA1         PAGE=1
    RFFTdata3   >   DATA1         PAGE=1
    RFFTdata4   >   DATA1         PAGE=1
    data:       >   DATA1         PAGE=1

    .ebss:      >   DATA1         PAGE=1
    .econst:    >   FLASH_B       PAGE=1

    /* required for printf */
//    .const:     >   FLASH_B  PAGE=1
//    .cio:       >   DATA1  PAGE=0

    codestart: >    BEGIN_FLASH, PAGE = 0, ALIGN(4)
   .reset:     >    RESET, PAGE = 0, TYPE = DSECT /* not used, */

    ramCode       : LOAD = FLASH_A,
                    RUN = PROG_MEM1,
                    LOAD_START(_ramCode_loadstart),
                    LOAD_SIZE(_ramCode_loadsize),
                    LOAD_END(_ramCode_loadend),
                    RUN_START(_ramCode_runstart),
                    RUN_SIZE(_ramCode_runsize),
                    RUN_END(_ramCode_runend),
                    PAGE = 0, ALIGN(4)

    ramConsts     : LOAD = FLASH_B,
                    RUN = DATA1,
                    LOAD_START(_ramConsts_loadstart),
                    LOAD_SIZE(_ramConsts_loadsize),
                    LOAD_END(_ramConsts_loadend),
                    RUN_START(_ramConsts_runstart),
                    RUN_SIZE(_ramConsts_runsize),
                    RUN_END(_ramConsts_runend),
                    PAGE = 1, ALIGN(4)

}
