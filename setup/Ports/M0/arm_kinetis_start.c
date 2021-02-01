/*
 *    (c) Copyright 2016 DDPA LLC
 *    ALL RIGHTS RESERVED.
 *
 *    Startup code for M0/M0+ processors.
 *
 *    Entry point for ARM Kinetis programs. This module is completely
 *    stand-alone and does not require any libraries except for writing
 *    fault information to the Flash.
 *
 *    Always compile this file with no optimization. There is a bug in the
 *    compiler with rtl epilogue generated for naked functions for optimization
 *    greater than -O1.
 *
 *    https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56732
 *
 */
#pragma GCC optimize ("O0")

#include "derivative.h"
#include "fault.h"

#if (__CORTEX_M != 0) /* Confirm processor type */
#error "Startup code targeted at M0 processors."
#endif


#define UNUSED_STACK_FILL  (0x55)

extern void main(void);

extern char __START_BSS[];
extern char __END_BSS[];
extern char __STACK_START[];
extern char __SP_INIT[];


void saveFaultState (uint32_t * stacked_args) {
    volatile struct fault_t  fault;

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

    __asm volatile ("bkpt");
    while (1);
}


/*
 * Redirect here for all unhandled processor faults.
 *
 * On an exception, the processor pushes onto the current stack.
 *
 * From http://embeddedgurus.com/state-space/tag/arm-cortex-m/
 * Test the stack pointer. If it has overflowed the stack area, reset
 * it to the top of the stack before proceeding.
 *
 * Put the active stack pointer (MSP or PSP) into r0, and call saveFaultState()
 * From http://mcuoneclipse.com/2012/11/24/debugging-hard-faults-on-arm-cortex-m/
 *
 * The fault state can then be examined in the debugger easily through
 * the predefined struct fault_t.
 *
 */
void Fault_Handler(void) __attribute__((naked));
void Fault_Handler(void) {
    __asm volatile (
            /* In handler mode, stack is MSP */
            "CPSID  i                                                        \n"
            "mov r0, sp                                                      \n"
            "ldr r1, =__STACK_START                                          \n"
            "cmp r0, r1                                                      \n"
            "bcs stack_ok                                                    \n"
            "ldr r0,=__SP_INIT                                               \n"
            "mov sp, r0                                                      \n"
        "stack_ok:                                                           \n"
            /* Test for active stack when exception occurred */
            " movs r0,#4                                                     \n"
            " movs r1, lr                                                    \n"
            " tst r0, r1                                                     \n"
            " beq _MSP                                                       \n"
            " mrs r0, psp                                                    \n"
            " b _HALT                                                        \n"
          "_MSP:                                                             \n"
            " mrs r0, msp                                                    \n"
          "_HALT:                                                            \n"
            "b saveFaultState\n"
    );
}

/*
 * Redirect here for all uninitialized vectors and assertions.
 * We could also vector directly to Fault_Handler()
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

    /*
     * Disable watchdog
     */
    #if ((defined MCU_MKL03Z4) || (defined MCU_MKL25Z4))
        SIM_COPC = 0x00;
    #else
        #error "Unrecognized processor type."
    #endif

    /*
     * Initialize the stack area for the stack sniffer. The stack area can
     * be filled all the way to the top because the stack pointer has not
     * been used yet. Symbols must be aligned to a 4 byte boundary.
     */
    __asm (
        "ldr r1, =__STACK_START\n"
        "ldr r2, =__SP_INIT\n"
        "ldr r0, =0x55555555\n"       // 0x55555555 = UNUSED_STACK_FILL
        "cmp     r2, r1\n"
        "ble     .LLC5\n"
    ".LLC3:\n"
        "str     r0, [r1]\n"
        "add     r1, #4\n"
        "cmp     r1, r2\n"
        "blt    .LLC3\n"
    ".LLC5:\n"
    );

    /*
     *  Zero out BSS section. Symbols must be aligned to a 4 byte boundary.
     */
    __asm volatile (
        "ldr r1, =__START_BSS\n"
        "ldr r2, =__END_BSS\n"
        "cmp     r2, r1\n"
        "ble     .LLC4\n"
        "movs    r0, #0\n"
    ".LLC2:\n"
        "str     r0, [r1]\n"
        "add     r1, #4\n"
        "cmp     r1, r2\n"
        "blt    .LLC2\n"
    ".LLC4:\n"
    );

    /*
     *  Copy data from read only memory to RAM. Symbols must be aligned to 4 byte boundary.
     */
    __asm volatile (
        ".syntax unified\n\t"   // required to make assembler recognize subs syntax
        "ldr    r1, =_etext\n"
        "ldr    r2, =_sdata\n"
        "ldr    r3, =_edata\n"
        "subs   r3, r2\n"
        "ble    .LLC1\n"
    ".LLC0:\n"
        "subs   r3, #4\n"
        "ldr    r0, [r1, r3]\n"
        "str    r0, [r2, r3]\n"
        "bgt    .LLC0\n"
    ".LLC1:\n"
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


#ifdef MCU_MKL03Z4
/*
 * Weak definitions of handlers point to Default_Handler if not implemented
 */
void NMI_Handler()            __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler()      __attribute__ ((weak, alias("Default_Handler")));
void SVC_Handler()            __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler()         __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler()        __attribute__ ((weak, alias("Default_Handler")));

void FTFA_IRQHandler()        __attribute__ ((weak, alias("Default_Handler")));
void PMC_IRQHandler()         __attribute__ ((weak, alias("Default_Handler")));
void LLWU_IRQHandler()        __attribute__ ((weak, alias("Default_Handler")));
void I2C0_IRQHandler()        __attribute__ ((weak, alias("Default_Handler")));
void SPI0_IRQHandler()        __attribute__ ((weak, alias("Default_Handler")));
void LPUART0_IRQHandler()     __attribute__ ((weak, alias("Default_Handler")));
void ADC0_IRQHandler()        __attribute__ ((weak, alias("Default_Handler")));
void CMP0_IRQHandler()        __attribute__ ((weak, alias("Default_Handler")));
void TPM0_IRQHandler()        __attribute__ ((weak, alias("Default_Handler")));
void TPM1_IRQHandler()        __attribute__ ((weak, alias("Default_Handler")));
void RTC_Alarm_IRQHandler()   __attribute__ ((weak, alias("Default_Handler")));
void RTC_Seconds_IRQHandler() __attribute__ ((weak, alias("Default_Handler")));
void LPTMR0_IRQHandler()      __attribute__ ((weak, alias("Default_Handler")));
void PORTA_IRQHandler()       __attribute__ ((weak, alias("Default_Handler")));
void PORTB_IRQHandler()       __attribute__ ((weak, alias("Default_Handler")));


/*
 * Interrupt Vector Table
 * */
void (* const InterruptVector[])() __attribute__ ((section(".vectortable"))) = {
    /* Processor exceptions */
    (void(*)(void)) __SP_INIT,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    SVC_Handler,
    Default_Handler,
    Default_Handler,
    PendSV_Handler,
    SysTick_Handler,

    /* Interrupts */
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    FTFA_IRQHandler,
    PMC_IRQHandler,
    LLWU_IRQHandler,
    I2C0_IRQHandler,
    Default_Handler,
    SPI0_IRQHandler,
    Default_Handler,
    LPUART0_IRQHandler,
    Default_Handler,
    Default_Handler,
    ADC0_IRQHandler,
    CMP0_IRQHandler,
    TPM0_IRQHandler,
    TPM1_IRQHandler,
    Default_Handler,
    RTC_Alarm_IRQHandler,
    RTC_Seconds_IRQHandler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    LPTMR0_IRQHandler,
    Default_Handler,
    PORTA_IRQHandler,
    PORTB_IRQHandler
};

#endif /* MCU_MKL03Z4 */


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

