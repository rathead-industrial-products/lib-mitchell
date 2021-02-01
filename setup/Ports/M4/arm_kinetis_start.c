/*******************************************************************************

    Entry point for ARM Kinetis programs. This module is completely
    stand-alone and does not require any libraries except for writing
    fault information to the Flash.

    Supported Processors:
    MCU_MKL25Z4
    MCU_MK22F25612
    MCU_MK22F51212


    Always compile this file with optimization -O1. There is a bug in the
    compiler with rtl epilogue generated for naked functions for optimization
    greater than -O1.

    https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56732

*******************************************************************************/

#pragma GCC optimize ("O1")


/* Configuration parameters defined as START_ENABLED or START_DISABLED */
#define START_ENABLED       1
#define START_DISABLED      0

#define START_USB_GASKET    START_DISABLED
#define START_WATCHDOG      START_DISABLED
#define START_SET_VTOR      START_DISABLED



#include <string.h>
#include "MK22F25612.h"
#include "fault.h"


typedef struct {
  char *        source;
  char *        target;
  unsigned long size;
} rom_info_t;

#define UNUSED_STACK_FILL  (0x55)

extern int main(void);

extern char __START_BSS[];
extern char __END_BSS[];
extern char __STACK_START[];
extern char __SP_INIT[];
extern char __APPLICATION_BASE[];
extern char __CORE_DUMP_START[];


#ifdef MCU_MKL25Z4
#define SCB   SystemControl_BASE_PTR
#endif


#if (START_USB_GASKET == START_ENABLED)

#include "gasket.h"

/*******************************************************************************
 *
 * Vector here for all exceptions that are handled in the bootloader,
 * i.e. USB. Program flow is redirected to the address in the bootloader
 * exception table.
 *
 ******************************************************************************/
void Gasket_Handler(void) __attribute__((naked));
void Gasket_Handler(void) {
    __asm volatile (
            "mrs r0, ipsr\n"        // interrupt program status register
            "lsl r0, #2\n"          // current exception number x sizeof(word)
            "ldr r0, [r0]\n"        // fetch bootloader exception vector
            "bx r0\n"               // jump to bootloader exception
    );

}




/******************************************************************************
 *
 * flash_err_t flashEraseProgramBlock(void * p_sector, void * p_block, uint32_t block_size);
 *
 * Starting at p_sector, erase enough sectors to fully contain block_size, then
 * program the area with the data starting at p_block.
 *
 * Copied from flash.c which is not included in this project - the flash
 * routines in the bootloader gasket are used instead.
 *
 *****************************************************************************/
flash_err_t flashEraseProgramBlock(void * p_sector, void * p_block, uint32_t block_size) {
    int sectors = (block_size / FLASH_SECTOR_SIZE) + 1;
    uint32_t * p_prog_dst = (uint32_t *) p_sector;
    uint32_t * p_prog_src = (uint32_t *) p_block;
    flash_err_t err =  FLASH_NO_ERR;

    while (sectors-- && (err == FLASH_NO_ERR)) {
        err = ee_flashEraseSector(p_sector);
        p_sector = (char *) p_sector + FLASH_SECTOR_SIZE;
    }

    block_size = (block_size + 3) & 0xfffffffc;   // ensure block_size is longword multiple
    while (block_size && (err == FLASH_NO_ERR)) {
        err = ee_flashProgramLongword((void *) p_prog_dst, *p_prog_src);
        ++p_prog_dst;
        ++p_prog_src;
        block_size -= 4;
    }

    return (err);
}

#endif    /* (START_USB_GASKET == START_ENABLED) */


void saveFaultState (uint32_t * stacked_args) {
    struct fault_t  fault;

    fault.stacked_registers.r0  = stacked_args[0];
    fault.stacked_registers.r1  = stacked_args[1];
    fault.stacked_registers.r2  = stacked_args[2];
    fault.stacked_registers.r3  = stacked_args[3];
    fault.stacked_registers.r12 = stacked_args[4];
    fault.stacked_registers.lr  = stacked_args[5];
    fault.stacked_registers.pc  = stacked_args[6];

    /* pc points to NEXT instruction to be executed */

    fault.psr.reg   = stacked_args[7];
    fault.icsr.reg  = SCB->ICSR;
    fault.shcsr.reg = SCB->SHCSR;
    #if (__CORTEX_M != 0) /* regs don't exist on M0 */
    fault.cfsr.reg  = SCB->CFSR;
    fault.bfar.reg  = SCB->BFAR;
    fault.afsr.reg  = SCB->AFSR;
    #endif


#if (START_USB_GASKET == START_ENABLED)

    /* Save the fault information in the flash for post-mortem analysis */

    /*
     * Reduce the clocks to 60/30/12 MHz by changing the ext reference divider
     * from 12 (48 / 12 = 4 MHz external refclk) to 24 (2 MHz ext refclk).
     * Put the MCU into RUN mode.
     */
    MCG_C5 = (MCG_C5 &~MCG_C5_PRDIV0_MASK) | MCG_C5_PRDIV0(23);
    SMC_PMCTRL = SMC_PMCTRL_RUNM(0);

    (void) flashEraseProgramBlock((void *) __CORE_DUMP_START, (void *) &fault, sizeof(fault));

    /* Don't bother restoring MCU to HSRUN */

#endif    /* (START_USB_GASKET == START_ENABLED) */

    __asm volatile ("bkpt");
    while (1);
}


/*
 * Redirect here for all unhandled processor faults.
 *
 * From http://embeddedgurus.com/state-space/tag/arm-cortex-m/
 * Test the stack pointer. If it has overflowed the stack area, reset
 * it to the top of the stack before proceeding.
 *
 * Put the active stack pointer (MSP or PSP) into r0, and call saveFaultState()
 *
 * The fault state can then be examined in the debugger easily through
 * the predefined struct fault_t.
 *
 */
void Fault_Handler(void) __attribute__((naked));
void Fault_Handler(void) {
    __asm volatile (
            "mov r0, sp\n"
            "ldr r1, =__STACK_START\n"
            "cmp r0, r1\n"
            "bcs stack_ok\n"
            "ldr sp, =__SP_INIT\n"
        "stack_ok:\n"
            "mrs r0, psp\n"   // default for m0 until I recode in m0 assy
            #if (__CORTEX_M != 0)
            "tst lr, #4\n"
            "ite eq\n"
            "mrseq r0, msp\n"
            "mrsne r0, psp\n"
            #endif
            "b saveFaultState\n"
    );
}

/*
 * Redirect here for all uninitialized vectors and assertions.
 *
 */
void Default_Handler(void) __attribute__((naked));
void Default_Handler(void) {
    Fault_Handler();
}


void Reset_Handler(void) __attribute__((naked));
void Reset_Handler(void)
{

    /* Disable interrupts */
    __asm volatile ("cpsid   i\n");


#if (START_WATCHDOG == START_DISABLED)
    #ifdef MCU_MKL25Z4
    SIM_COPC = 0x00;
    #elif defined MCU_MK22F25612
    WDOG->UNLOCK  = WDOG_UNLOCK_WDOGUNLOCK(0xC520); /* Key 1 */
    WDOG->UNLOCK  = WDOG_UNLOCK_WDOGUNLOCK(0xD928); /* Key 2 */
    WDOG->STCTRLH = WDOG->STCTRLH & ~WDOG_STCTRLH_WDOGEN_MASK;  /* set WDOGEN bit to 0 */
    #else
    #error ("Cannot disable watchdog - unknown MCU type.")
    #endif
#endif   /* (START_WATCHDOG == START_DISABLED) */


    /*
     * Set the vector base register
     */
#if (START_SET_VTOR == START_ENABLED)
    SCB->VTOR = (uint32_t) __APPLICATION_BASE;
#endif   /* (START_VTOR == START_ENABLED) */


    /*
     * Initialize the stack area for the stack sniffer. The stack area can
     * be filled all the way to the top because the stack pointer has not
     * been used yet. Symbols must be aligned to a 4 byte boundary.
     */
    __asm (
        "ldr r1, =__STACK_START\n"
        "ldr r2, =__SP_INIT\n"
        "ldr r0, =0x55555555\n"       // 0x55555555 = UNUSED_STACK_FILL
    ".LC3:\n"
        "cmp     r1, r2\n"
        "itt     lt\n"
        "strlt   r0, [r1], #4\n"
        "blt    .LC3\n"
    );

    /*
     *  Zero out BSS section. Symbols must be aligned to a 4 byte boundary.
     */
    __asm volatile (
        "ldr r1, =__START_BSS\n"
        "ldr r2, =__END_BSS\n"
        "movs    r0, 0\n"
    ".LC2:\n"
        "cmp     r1, r2\n"
        "itt    lt\n"
        "strlt   r0, [r1], #4\n"
        "blt    .LC2\n"
    );

    /*
     *  Copy data from read only memory to RAM. Symbols must be aligned to 4 byte boundary.
     */
    __asm volatile (
        "ldr    r1, =_etext\n"
        "ldr    r2, =_sdata\n"
        "ldr    r3, =_edata\n"
        "subs    r3, r2\n"
        "ble    .LC1\n"
    ".LC0:\n"
        "subs    r3, #4\n"
        "ldr    r0, [r1, r3]\n"
        "str    r0, [r2, r3]\n"
        "bgt    .LC0\n"
    ".LC1:\n"
    );

    //  call main
    #if (UNIT_TEST)
    __asm (
        "ldr r0, =unit_test_main\n"
        "mov pc,r0\n"
        );
    #else
    __asm (
        "ldr r0, =main\n"
        "mov pc,r0\n"
        );
    #endif /* UNIT TEST */
}


#if ((defined MCU_MK22F51212) || (defined MCU_MK22F25612))

/*
 * Weak definitions of handlers point to Default_Handler if not implemented
 */
//void Reset_Handler()              __attribute__ ((weak, alias("Fault_Handler")));
void NMI_Handler()                __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler()          __attribute__ ((weak, alias("Fault_Handler")));
void MemManage_Handler()          __attribute__ ((weak, alias("Fault_Handler")));
void BusFault_Handler()           __attribute__ ((weak, alias("Fault_Handler")));
void UsageFault_Handler()         __attribute__ ((weak, alias("Fault_Handler")));
void SVC_Handler()                __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler()           __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler()             __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler()            __attribute__ ((weak, alias("Default_Handler")));

void DMA0_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void DMA2_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void DMA3_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void DMA4_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void DMA5_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void DMA6_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void DMA7_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void DMA8_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void DMA9_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void DMA10_IRQHandler()           __attribute__ ((weak, alias("Default_Handler")));
void DMA11_IRQHandler()           __attribute__ ((weak, alias("Default_Handler")));
void DMA12_IRQHandler()           __attribute__ ((weak, alias("Default_Handler")));
void DMA13_IRQHandler()           __attribute__ ((weak, alias("Default_Handler")));
void DMA14_IRQHandler()           __attribute__ ((weak, alias("Default_Handler")));
void DMA15_IRQHandler()           __attribute__ ((weak, alias("Default_Handler")));
void DMA_Error_IRQHandler()       __attribute__ ((weak, alias("Default_Handler")));
void MCM_IRQHandler()             __attribute__ ((weak, alias("Default_Handler")));
void FTF_IRQHandler()             __attribute__ ((weak, alias("Default_Handler")));
void Read_Collision_IRQHandler()  __attribute__ ((weak, alias("Default_Handler")));
void LVD_LVW_IRQHandler()         __attribute__ ((weak, alias("Default_Handler")));
void LLW_IRQHandler()             __attribute__ ((weak, alias("Default_Handler")));
void Watchdog_IRQHandler()        __attribute__ ((weak, alias("Default_Handler")));
void RNG_IRQHandler()             __attribute__ ((weak, alias("Default_Handler")));
void I2C0_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void I2C1_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void SPI0_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void SPI1_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void I2S0_Tx_IRQHandler()         __attribute__ ((weak, alias("Default_Handler")));
void I2S0_Rx_IRQHandler()         __attribute__ ((weak, alias("Default_Handler")));
void LPUART0_IRQHandler()         __attribute__ ((weak, alias("Default_Handler")));
void UART0_RX_TX_IRQHandler()     __attribute__ ((weak, alias("Default_Handler")));
void UART0_ERR_IRQHandler()       __attribute__ ((weak, alias("Default_Handler")));
void UART1_RX_TX_IRQHandler()     __attribute__ ((weak, alias("Default_Handler")));
void UART1_ERR_IRQHandler()       __attribute__ ((weak, alias("Default_Handler")));
void UART2_RX_TX_IRQHandler()     __attribute__ ((weak, alias("Default_Handler")));
void UART2_ERR_IRQHandler()       __attribute__ ((weak, alias("Default_Handler")));
void ADC0_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void CMP0_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void CMP1_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void FTM0_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void FTM1_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void FTM2_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void RTC_IRQHandler()             __attribute__ ((weak, alias("Default_Handler")));
void RTC_Seconds_IRQHandler()     __attribute__ ((weak, alias("Default_Handler")));
void PIT0_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void PIT1_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void PIT2_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void PIT3_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void PDB0_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
#if (START_USB_GASKET == START_ENABLED)
void USB0_IRQHandler()            __attribute__ ((weak, alias("Gasket_Handler")));
#else
void USB0_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
#endif
void DAC0_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));
void MCG_IRQHandler()             __attribute__ ((weak, alias("Default_Handler")));
void LPTimer_IRQHandler()         __attribute__ ((weak, alias("Default_Handler")));
void PORTA_IRQHandler()           __attribute__ ((weak, alias("Default_Handler")));
void PORTB_IRQHandler()           __attribute__ ((weak, alias("Default_Handler")));
void PORTC_IRQHandler()           __attribute__ ((weak, alias("Default_Handler")));
void PORTD_IRQHandler()           __attribute__ ((weak, alias("Default_Handler")));
void PORTE_IRQHandler()           __attribute__ ((weak, alias("Default_Handler")));
void SWI_IRQHandler()             __attribute__ ((weak, alias("Default_Handler")));
void ADC1_IRQHandler()            __attribute__ ((weak, alias("Default_Handler")));


/*
 * Interrupt Vector Table
 *
 */
void (* const InterruptVector[])() __attribute__ ((section(".vectortable"))) = {
    (void(*)(void)) __SP_INIT,                       /* Top of Stack */
    Reset_Handler,                                   /* Reset Handler */
    NMI_Handler,                                     /* NMI Handler*/
    HardFault_Handler,                               /* Hard Fault Handler*/
    MemManage_Handler,                               /* MPU Fault Handler*/
    BusFault_Handler,                                /* Bus Fault Handler*/
    UsageFault_Handler,                              /* Usage Fault Handler*/
    Default_Handler,                                 /* Reserved*/
    Default_Handler,                                 /* Reserved*/
    Default_Handler,                                 /* Reserved*/
    Default_Handler,                                 /* Reserved*/
    SVC_Handler,                                     /* SVCall Handler*/
    DebugMon_Handler,                                /* Debug Monitor Handler*/
    Default_Handler,                                 /* Reserved*/
    PendSV_Handler,                                  /* PendSV Handler*/
    SysTick_Handler,                                 /* SysTick Handler*/

    DMA0_IRQHandler,                                 /* DMA Channel 0 Transfer Complete*/
    DMA1_IRQHandler,                                 /* DMA Channel 1 Transfer Complete*/
    DMA2_IRQHandler,                                 /* DMA Channel 2 Transfer Complete*/
    DMA3_IRQHandler,                                 /* DMA Channel 3 Transfer Complete*/
    DMA4_IRQHandler,                                 /* DMA Channel 4 Transfer Complete*/
    DMA5_IRQHandler,                                 /* DMA Channel 5 Transfer Complete*/
    DMA6_IRQHandler,                                 /* DMA Channel 6 Transfer Complete*/
    DMA7_IRQHandler,                                 /* DMA Channel 7 Transfer Complete*/
    DMA8_IRQHandler,                                 /* DMA Channel 8 Transfer Complete*/
    DMA9_IRQHandler,                                 /* DMA Channel 9 Transfer Complete*/
    DMA10_IRQHandler,                                /* DMA Channel 10 Transfer Complete*/
    DMA11_IRQHandler,                                /* DMA Channel 11 Transfer Complete*/
    DMA12_IRQHandler,                                /* DMA Channel 12 Transfer Complete*/
    DMA13_IRQHandler,                                /* DMA Channel 13 Transfer Complete*/
    DMA14_IRQHandler,                                /* DMA Channel 14 Transfer Complete*/
    DMA15_IRQHandler,                                /* DMA Channel 15 Transfer Complete*/
    DMA_Error_IRQHandler,                            /* DMA Error Interrupt*/
    MCM_IRQHandler,                                  /* Normal Interrupt*/
    FTF_IRQHandler,                                  /* FTFA Command complete interrupt*/
    Read_Collision_IRQHandler,                       /* Read Collision Interrupt*/
    LVD_LVW_IRQHandler,                              /* Low Voltage Detect, Low Voltage Warning*/
    LLW_IRQHandler,                                  /* Low Leakage Wakeup*/
    Watchdog_IRQHandler,                             /* WDOG Interrupt*/
    RNG_IRQHandler,                                  /* RNG Interrupt*/
    I2C0_IRQHandler,                                 /* I2C0 interrupt*/
    I2C1_IRQHandler,                                 /* I2C1 interrupt*/
    SPI0_IRQHandler,                                 /* SPI0 Interrupt*/
    SPI1_IRQHandler,                                 /* SPI1 Interrupt*/
    I2S0_Tx_IRQHandler,                              /* I2S0 transmit interrupt*/
    I2S0_Rx_IRQHandler,                              /* I2S0 receive interrupt*/
    LPUART0_IRQHandler,                              /* LPUART0 status/error interrupt*/
    UART0_RX_TX_IRQHandler,                          /* UART0 Receive/Transmit interrupt*/
    UART0_ERR_IRQHandler,                            /* UART0 Error interrupt*/
    UART1_RX_TX_IRQHandler,                          /* UART1 Receive/Transmit interrupt*/
    UART1_ERR_IRQHandler,                            /* UART1 Error interrupt*/
    UART2_RX_TX_IRQHandler,                          /* UART2 Receive/Transmit interrupt*/
    UART2_ERR_IRQHandler,                            /* UART2 Error interrupt*/
    Default_Handler,                                 /* Reserved*/
    Default_Handler,                                 /* Reserved*/
    ADC0_IRQHandler,                                 /* ADC0 interrupt*/
    CMP0_IRQHandler,                                 /* CMP0 interrupt*/
    CMP1_IRQHandler,                                 /* CMP1 interrupt*/
    FTM0_IRQHandler,                                 /* FTM0 fault, overflow and channels interrupt*/
    FTM1_IRQHandler,                                 /* FTM1 fault, overflow and channels interrupt*/
    FTM2_IRQHandler,                                 /* FTM2 fault, overflow and channels interrupt*/
    Default_Handler,                                 /* Reserved*/
    RTC_IRQHandler,                                  /* RTC interrupt*/
    RTC_Seconds_IRQHandler,                          /* RTC seconds interrupt*/
    PIT0_IRQHandler,                                 /* PIT timer channel 0 interrupt*/
    PIT1_IRQHandler,                                 /* PIT timer channel 1 interrupt*/
    PIT2_IRQHandler,                                 /* PIT timer channel 2 interrupt*/
    PIT3_IRQHandler,                                 /* PIT timer channel 3 interrupt*/
    PDB0_IRQHandler,                                 /* PDB0 Interrupt*/
    USB0_IRQHandler,                                 /* USB0 interrupt*/
    Default_Handler,                                 /* Reserved*/
    Default_Handler,                                 /* Reserved*/
    DAC0_IRQHandler,                                 /* DAC0 interrupt*/
    MCG_IRQHandler,                                  /* MCG Interrupt*/
    LPTimer_IRQHandler,                              /* LPTimer interrupt*/
    PORTA_IRQHandler,                                /* Port A interrupt*/
    PORTB_IRQHandler,                                /* Port B interrupt*/
    PORTC_IRQHandler,                                /* Port C interrupt*/
    PORTD_IRQHandler,                                /* Port D interrupt*/
    PORTE_IRQHandler,                                /* Port E interrupt*/
    SWI_IRQHandler,                                  /* Software interrupt*/
    Default_Handler,                                 /* Reserved*/
    Default_Handler,                                 /* Reserved*/
    Default_Handler,                                 /* Reserved*/
    Default_Handler,                                 /* Reserved*/
    Default_Handler,                                 /* Reserved*/
    Default_Handler,                                 /* Reserved*/
    Default_Handler,                                 /* Reserved*/
    Default_Handler,                                 /* Reserved*/
    ADC1_IRQHandler                                  /* ADC1 interrupt*/
};

#endif /* ((defined MCU_MK22F51212) || (defined MCU_MK22F25612)) */

#ifdef MCU_MKL25Z4
/*
 * Weak definitions of handlers point to Default_Handler if not implemented
 */
void NMI_Handler() __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler() __attribute__ ((weak, alias("Default_Handler")));
void SVC_Handler() __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler() __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler() __attribute__ ((weak, alias("Default_Handler")));

void DMA0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void DMA1_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void DMA2_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void DMA3_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void MCM_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void FTFL_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void PMC_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void LLW_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void I2C0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void I2C1_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void SPI0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void SPI1_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void UART0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void UART1_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void UART2_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void ADC0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void CMP0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void FTM0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void FTM1_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void FTM2_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void RTC_Alarm_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void RTC_Seconds_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void PIT_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void USBOTG_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void DAC0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void TSI0_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void MCG_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void LPTimer_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void PORTA_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void PORTD_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));


/*
 * Interrupt Vector Table
 * */
void (* const InterruptVector[])() __attribute__ ((section(".vectortable"))) = {
    /* Processor exceptions */
    (void(*)(void)) __SP_INIT,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    SVC_Handler,
    0,
    0,
    PendSV_Handler,
    SysTick_Handler,

    /* Interrupts */
    DMA0_IRQHandler, /* DMA Channel 0 Transfer Complete and Error */
    DMA1_IRQHandler, /* DMA Channel 1 Transfer Complete and Error */
    DMA2_IRQHandler, /* DMA Channel 2 Transfer Complete and Error */
    DMA3_IRQHandler, /* DMA Channel 3 Transfer Complete and Error */
    MCM_IRQHandler, /* Normal Interrupt */
    FTFL_IRQHandler, /* FTFL Interrupt */
    PMC_IRQHandler, /* PMC Interrupt */
    LLW_IRQHandler, /* Low Leakage Wake-up */
    I2C0_IRQHandler, /* I2C0 interrupt */
    I2C1_IRQHandler, /* I2C1 interrupt */
    SPI0_IRQHandler, /* SPI0 Interrupt */
    SPI1_IRQHandler, /* SPI1 Interrupt */
    UART0_IRQHandler, /* UART0 Status and Error interrupt */
    UART1_IRQHandler, /* UART1 Status and Error interrupt */
    UART2_IRQHandler, /* UART2 Status and Error interrupt */
    ADC0_IRQHandler, /* ADC0 interrupt */
    CMP0_IRQHandler, /* CMP0 interrupt */
    FTM0_IRQHandler, /* FTM0 fault, overflow and channels interrupt */
    FTM1_IRQHandler, /* FTM1 fault, overflow and channels interrupt */
    FTM2_IRQHandler, /* FTM2 fault, overflow and channels interrupt */
    RTC_Alarm_IRQHandler, /* RTC Alarm interrupt */
    RTC_Seconds_IRQHandler, /* RTC Seconds interrupt */
    PIT_IRQHandler, /* PIT timer all channels interrupt */
    Default_Handler, /* Reserved interrupt 39/23 */
    USBOTG_IRQHandler, /* USB interrupt */
    DAC0_IRQHandler, /* DAC0 interrupt */
    TSI0_IRQHandler, /* TSI0 Interrupt */
    MCG_IRQHandler, /* MCG Interrupt */
    LPTimer_IRQHandler, /* LPTimer interrupt */
    Default_Handler, /* Reserved interrupt 45/29 */
    PORTA_IRQHandler, /* Port A interrupt */
    PORTD_IRQHandler /* Port D interrupt */
};

#endif /* MCU_MKL25Z4 */

