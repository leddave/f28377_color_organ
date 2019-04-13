// See F2807x_Headers__nonBIOS.cmd for all register areas
-x
-stack 0x400
-heap  0x100

// F28377 Memory Layout:
// Dedicated RAM (CPU Only):    0x000000 - 0x0007ff    (1K * 2:  M0, M1)
// Message RAM (CLA -> CPU):    0x001480 - 0x0014ff    (128B, cla write, cpu read)
//             (CPU -> CLA):    0x001500 - 0x00157f    (128B, cpu write, cla read)
// UPP Msg RAM (Tx):            0x006c00 - 0x006dff    (512B)
//             (Rx):            0x006e00 - 0x006fff    (512B)
// LS0..5 RAM:                  0x008000 - 0x00afff    (2KB * 6, cla r/w)
// D0..1 RAM:                   0x00b000 - 0x00bfff    (2KB * 2)
// GS0..15 RAM:                 0x00c000 - 0x01bfff    (4KB * 16)
// CAN A RAM:                   0x049000 - 0x0497ff    (2KB)
// CAN B RAM:                   0x04b000 - 0x04b7ff    (2KB)
// Flash:                       0x080000 - 0x0fffff    (256KB * 2)
// Boot ROM:                    0x3f8000 - 0x3fffbf    (32K-64B)
// Vectors:                     0x3fffc0 - 0x3fffff    (64B)

// Define a size for the CLA scratchpad area that will be used
// by the CLA compiler for local symbols and temps
// Also force references to the special symbols that mark the
// scratchpad are.
CLA_SCRATCHPAD_SIZE = 0x100;
--undef_sym=__cla_scratchpad_end
--undef_sym=__cla_scratchpad_start

MEMORY
{
    PAGE 0:
    PROG_MEM0   :   origin = 0x00000122  length = 0x000002DE
    UPP_RAM     :   origin = 0x00006c00  length = 0x00000400
    RAMD01      :   origin = 0x0000B000, length = 0x00001000
    RAMLS5      :   origin = 0x0000A800, length = 0x00000800
    RAM_GLB     :   origin = 0x0000c000  length = 0x00010000
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
    PIE_CTRL    :   origin = 0x00000CE0  length = 0x00000020     /* PIE control registers */
    PIE_VECT    :   origin = 0x00000D00  length = 0x00000200     /* PIE Vector Table */
    CLA1_REGS   :   origin = 0x00001400, length = 0x00000080     /* CLA registers */
    EPWM1_REGS  :   origin = 0x00004000  length = 0x00000100
    WD_REGS     :   origin = 0x00007000  length = 0x00000040
    SCIA_REGS   :   origin = 0x00007200  length = 0x00000010
    SCIB_REGS   :   origin = 0x00007210  length = 0x00000010
    ADCA_REGS   :   origin = 0x00007400  length = 0x00000080
    ADCB_REGS   :   origin = 0x00007480  length = 0x00000080
    ADCC_REGS   :   origin = 0x00007500  length = 0x00000080
    ADCD_REGS   :   origin = 0x00007580  length = 0x00000080
    GPIO_CTRL   :   origin = 0x00007c00  length = 0x00000180
    GPIO_DATA   :   origin = 0x00007f00  length = 0x00000030
    INPUT_XBAR  :   origin = 0x00007900  length = 0x00000040
    RAMLS0123   :   origin = 0x00008000, length = 0x00002000
    RAMLS4      :   origin = 0x0000a000, length = 0x00000800
    DATA1       :   origin = 0x00018000  length = 0x00004000

    FLASH_PUMP  :   origin = 0x00050024  length = 0x00000002
    DEV_CFG     :   origin = 0x0005D000  length = 0x00000180
    ANALOG_SUBSYS : origin = 0x0005D180  length = 0x00000080
    CPU_SYS     :   origin = 0x0005D300  length = 0x00000100
    CLK_CFG     :   origin = 0x0005D200  length = 0x00000100
    MEMCFG      :   origin = 0x0005F400  length = 0x00000080  /* Mem Config regs */
    FLASH0_CTRL :   origin = 0x0005F800  length = 0x00000300
    FLASH0_ECC  :   origin = 0x0005FB00  length = 0x00000040
    FLASH1_CTRL :   origin = 0x0005Fc00  length = 0x00000300
    FLASH1_ECC  :   origin = 0x0005Ff00  length = 0x00000040
    BOOT_ROM    :   origin = 0x003f8000  length = 0x00007fbe
    VECTORS     :   origin = 0x003fffbe  length = 0x00000042

    CLA2CPU_MSGRAM : origin = 0x00001480 length = 0x00000080
    CPU2CLA_MSGRAM : origin = 0x00001500 length = 0x00000080
}

SECTIONS
{
    /* PAGE 0 */
    .text:      >   RAM_GLB       PAGE=0
    .cinit:     >   RAM_GLB       PAGE=0
    .reset:     >   RESET,        PAGE=0, TYPE = DSECT /* not used */
    .heap:      >   CANB_RAM      PAGE=0
    .switch:    >   RAM_GLB       PAGE=0

    /* PAGE 1 */
    .stack:     >   STACK         PAGE=1
    .system:    >   DATA1         PAGE=1
    .bss:       >   DATA1         PAGE=1
    .sysmem:    >   DATA1         PAGE=1

    .AdcaResult: >  ADCA_RESULT   PAGE=1
    .AdcbResult: >  ADCB_RESULT   PAGE=1
    .Timer0Ctrl: >  CPU_TIMER0    PAGE=1
    .Timer1Ctrl: >  CPU_TIMER1    PAGE=1
    .Timer2Ctrl: >  CPU_TIMER2    PAGE=1
    .GpioCtrl:  >   GPIO_CTRL     PAGE=1
    .GpioData:  >   GPIO_DATA     PAGE=1
    .Epwm1Reg:  >   EPWM1_REGS    PAGE=1
    .WdReg:     >   WD_REGS       PAGE=1
    .SciaCfg:   >   SCIA_REGS     PAGE=1
    .ScibCfg:   >   SCIB_REGS     PAGE=1
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

    .ClaRegs:    >  CLA1_REGS     PAGE = 1
    .MemCfgRegs: >  MEMCFG        PAGE = 1

    .ebss:      >   DATA1         PAGE=1
    .econst:    >   DATA1         PAGE=1

    codestart:  >   RAM_GLB       PAGE=0
    ramCode:    >   RAM_GLB       PAGE=0
    claCode:    >   RAMLS5        PAGE=0
    ramConsts:  >   DATA1         PAGE=1
    leddata:    >   RAMLS0123     PAGE=1
    data:       >   DATA1         PAGE=1

    /* CLA specific sections */
   Cla1Prog         : > RAMLS5, PAGE=0

   Cla1ToCpuMsgRAM  : > CLA2CPU_MSGRAM  PAGE = 1
   CpuToCla1MsgRAM  : > CPU2CLA_MSGRAM  PAGE = 1

   /* CLA C compiler sections */
   //
   // Must be allocated to memory the CLA has write access to
   //
   CLAscratch       :
                     { *.obj(CLAscratch)
                     . += CLA_SCRATCHPAD_SIZE;
                     *.obj(CLAscratch_end) } >  RAMLS4,  PAGE = 1

   .scratchpad      : > RAMLS4,       PAGE = 1
   .bss_cla         : > RAMLS4,       PAGE = 1
   .const_cla       : > RAMLS4,       PAGE = 1

    /* required for printf */
//    .const:     >   DATA1  PAGE=0
//    .cio:       >   DATA1  PAGE=0
}