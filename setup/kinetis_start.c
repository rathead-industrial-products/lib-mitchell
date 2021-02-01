/*
 *    (c) Copyright 2016 DDPA LLC
 *    ALL RIGHTS RESERVED.
 *
 *    Startup code for M0/M0+/M4 processors.
 *
 *    Entry point for ARM Kinetis programs. This module is completely
 *    stand-alone and does not require any libraries except for writing
 *    fault information to the Flash.
 *
 *    How to trace a fault:
 *      Check fault.psr.cur_exception_num to determine exception number (16 is the first user interrupt).
 *
 *    Always compile this file with no optimization. There is a bug in the
 *    compiler with rtl epilogue generated for naked functions for optimization
 *    greater than -O1.
 *
 *    https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56732
 *
 *    Update: reported now as fixed
 *
 */
//#pragma GCC optimize ("O0")

#include "derivative.h"
#include "fault.h"


#define UNUSED_STACK_FILL  (0x55)

extern void main(void);


/* Linker symbols */
extern char __START_BSS[];
extern char __END_BSS[];
extern char __START_DATA[];
extern char __END_DATA[];
extern char __START_EIR[];
extern char __END_EIR[];
extern char __END_TEXT[];
extern char __END_TEXT_AND_DATA[];
extern char __STACK_START[];
extern char __SP_INIT[];


void saveFaultState (uint32_t * stacked_args) {
    volatile struct fault_t  fault;

    (void) fault;       // silences "set but not used" warning

    fault.stacked_registers.r0  = stacked_args[0];
    fault.stacked_registers.r1  = stacked_args[1];
    fault.stacked_registers.r2  = stacked_args[2];
    fault.stacked_registers.r3  = stacked_args[3];
    fault.stacked_registers.r12 = stacked_args[4];
    fault.stacked_registers.lr  = stacked_args[5];
    fault.stacked_registers.sp  = (uint32_t) stacked_args;
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

    __asm volatile ("bkpt");
    while (1);
}


/*
 * Redirect here for all faults and unhandled processor exceptions.
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


void Reset_Handler(void) __attribute__((naked));
void Reset_Handler(void)
{
    /* Disable interrupts */
    __asm volatile ("cpsid   i\n");

    /*
     * Disable watchdog
     */
    #if ((defined MCU_MKL03Z4) || (defined MCU_MKL25Z4) || (defined MCU_MKL43Z4))
        SIM_COPC = 0x00;
    #elif (defined MCU_MK22F25612)
        WDOG->UNLOCK  = WDOG_UNLOCK_WDOGUNLOCK(0xC520);             /* Key 1 */
        WDOG->UNLOCK  = WDOG_UNLOCK_WDOGUNLOCK(0xD928);             /* Key 2 */
        WDOG->STCTRLH = WDOG->STCTRLH & ~WDOG_STCTRLH_WDOGEN_MASK;  /* set WDOGEN bit to 0 */
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
        "ldr    r1, =__END_TEXT_AND_DATA\n"
        "ldr    r2, =__START_EIR\n"
        "ldr    r3, =__END_EIR\n"
        "subs   r3, r2\n"
        "ble    .LLC1\n"
    ".LLC0:\n"
        "subs   r3, #4\n"
        "ldr    r0, [r1, r3]\n"
        "str    r0, [r2, r3]\n"
        "bgt    .LLC0\n"
    ".LLC1:\n"
    );

    /*
     *  Copy code from read only memory to execute-in-RAM. Symbols must be aligned to 4 byte boundary.
     */
    __asm volatile (
        ".syntax unified\n\t"   // required to make assembler recognize subs syntax
        "ldr    r1, =__END_TEXT\n"
        "ldr    r2, =__START_DATA\n"
        "ldr    r3, =__END_DATA\n"
        "subs   r3, r2\n"
        "ble    .LLC7\n"
    ".LLC6:\n"
        "subs   r3, #4\n"
        "ldr    r0, [r1, r3]\n"
        "str    r0, [r2, r3]\n"
        "bgt    .LLC6\n"
    ".LLC7:\n"
    );

    //  call main
    __asm (
        "ldr r0, =main\n"
        "mov pc,r0\n"
        );
}


/* Weak definitions of handlers point to Fault_Handler if not implemented */
void NMI_Handler()                __attribute__ ((weak, alias("Fault_Handler")));
void HardFault_Handler()          __attribute__ ((weak, alias("Fault_Handler")));
void MemManage_Handler()          __attribute__ ((weak, alias("Fault_Handler")));
void BusFault_Handler()           __attribute__ ((weak, alias("Fault_Handler")));
void UsageFault_Handler()         __attribute__ ((weak, alias("Fault_Handler")));
void SVC_Handler()                __attribute__ ((weak, alias("Fault_Handler")));
void DebugMon_Handler()           __attribute__ ((weak, alias("Fault_Handler")));
void PendSV_Handler()             __attribute__ ((weak, alias("Fault_Handler")));
void SysTick_Handler()            __attribute__ ((weak, alias("Fault_Handler")));

void ADC0_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void ADC1_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void CMP0_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void CMP1_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void DAC0_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void DMA0_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void DMA1_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void DMA2_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void DMA3_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void DMA4_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void DMA5_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void DMA6_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void DMA7_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void DMA8_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void DMA9_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void DMA10_IRQHandler()           __attribute__ ((weak, alias("Fault_Handler")));
void DMA11_IRQHandler()           __attribute__ ((weak, alias("Fault_Handler")));
void DMA12_IRQHandler()           __attribute__ ((weak, alias("Fault_Handler")));
void DMA13_IRQHandler()           __attribute__ ((weak, alias("Fault_Handler")));
void DMA14_IRQHandler()           __attribute__ ((weak, alias("Fault_Handler")));
void DMA15_IRQHandler()           __attribute__ ((weak, alias("Fault_Handler")));
void DMA_Error_IRQHandler()       __attribute__ ((weak, alias("Fault_Handler")));
void FTF_IRQHandler()             __attribute__ ((weak, alias("Fault_Handler")));
void FTFA_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void FTFL_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void FTM0_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void FTM1_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void FTM2_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void I2C0_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void I2C1_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void I2S0_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void I2S0_Tx_IRQHandler()         __attribute__ ((weak, alias("Fault_Handler")));
void I2S0_Rx_IRQHandler()         __attribute__ ((weak, alias("Fault_Handler")));
void LCD_IRQHandler()             __attribute__ ((weak, alias("Fault_Handler")));
void LLW_IRQHandler()             __attribute__ ((weak, alias("Fault_Handler")));
void LPTMR0_IRQHandler()          __attribute__ ((weak, alias("Fault_Handler")));
void LPUART0_IRQHandler()         __attribute__ ((weak, alias("Fault_Handler")));
void LPUART1_IRQHandler()         __attribute__ ((weak, alias("Fault_Handler")));
void LVD_LVW_IRQHandler()         __attribute__ ((weak, alias("Fault_Handler")));
void MCG_IRQHandler()             __attribute__ ((weak, alias("Fault_Handler")));
void MCM_IRQHandler()             __attribute__ ((weak, alias("Fault_Handler")));
void PDB0_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void PIT_IRQHandler()             __attribute__ ((weak, alias("Fault_Handler")));
void PIT0_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void PIT1_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void PIT2_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void PIT3_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void PMC_IRQHandler()             __attribute__ ((weak, alias("Fault_Handler")));
void PORTA_IRQHandler()           __attribute__ ((weak, alias("Fault_Handler")));
void PORTB_IRQHandler()           __attribute__ ((weak, alias("Fault_Handler")));
void PORTC_IRQHandler()           __attribute__ ((weak, alias("Fault_Handler")));
void PORTCD_IRQHandler()          __attribute__ ((weak, alias("Fault_Handler")));
void PORTD_IRQHandler()           __attribute__ ((weak, alias("Fault_Handler")));
void PORTE_IRQHandler()           __attribute__ ((weak, alias("Fault_Handler")));
void Read_Collision_IRQHandler()  __attribute__ ((weak, alias("Fault_Handler")));
void RTC_IRQHandler()             __attribute__ ((weak, alias("Fault_Handler")));
void RTC_Alarm_IRQHandler()       __attribute__ ((weak, alias("Fault_Handler")));
void RTC_Seconds_IRQHandler()     __attribute__ ((weak, alias("Fault_Handler")));
void SPI0_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void SPI1_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void SWI_IRQHandler()             __attribute__ ((weak, alias("Fault_Handler")));
void TPM0_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void TPM1_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void TPM2_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void TSI0_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void RNG_IRQHandler()             __attribute__ ((weak, alias("Fault_Handler")));
void UART0_ERR_IRQHandler()       __attribute__ ((weak, alias("Fault_Handler")));
void UART0_IRQHandler()           __attribute__ ((weak, alias("Fault_Handler")));
void UART0_RX_TX_IRQHandler()     __attribute__ ((weak, alias("Fault_Handler")));
void UART1_ERR_IRQHandler()       __attribute__ ((weak, alias("Fault_Handler")));
void UART1_IRQHandler()           __attribute__ ((weak, alias("Fault_Handler")));
void UART1_RX_TX_IRQHandler()     __attribute__ ((weak, alias("Fault_Handler")));
void UART2_ERR_IRQHandler()       __attribute__ ((weak, alias("Fault_Handler")));
void UART2_IRQHandler()           __attribute__ ((weak, alias("Fault_Handler")));
void UART2_RX_TX_IRQHandler()     __attribute__ ((weak, alias("Fault_Handler")));
void UART2_FLEXIO_IRQHandler()    __attribute__ ((weak, alias("Fault_Handler")));
void USB0_IRQHandler()            __attribute__ ((weak, alias("Fault_Handler")));
void USBOTG_IRQHandler()          __attribute__ ((weak, alias("Fault_Handler")));
void Watchdog_IRQHandler()        __attribute__ ((weak, alias("Fault_Handler")));


#ifdef MCU_MKL03Z4
void (* const InterruptVector[])() __attribute__ ((section(".vectortable"))) = {
    (void(*)(void)) __SP_INIT,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    SVC_Handler,
    Fault_Handler,
    Fault_Handler,
    PendSV_Handler,
    SysTick_Handler,

    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    FTFA_IRQHandler,
    PMC_IRQHandler,
    LLW_IRQHandler,
    I2C0_IRQHandler,
    Fault_Handler,
    SPI0_IRQHandler,
    Fault_Handler,
    LPUART0_IRQHandler,
    Fault_Handler,
    Fault_Handler,
    ADC0_IRQHandler,
    CMP0_IRQHandler,
    TPM0_IRQHandler,
    TPM1_IRQHandler,
    Fault_Handler,
    RTC_Alarm_IRQHandler,
    RTC_Seconds_IRQHandler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    LPTMR0_IRQHandler,
    Fault_Handler,
    PORTA_IRQHandler,
    PORTB_IRQHandler
};
#endif /* MCU_MKL03Z4 */


#ifdef MCU_MKL25Z4
void (* const InterruptVector[])() __attribute__ ((section(".vectortable"))) = {
    (void(*)(void)) __SP_INIT,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    SVC_Handler,
    Fault_Handler,
    Fault_Handler,
    PendSV_Handler,
    SysTick_Handler,

    DMA0_IRQHandler,
    DMA1_IRQHandler,
    DMA2_IRQHandler,
    DMA3_IRQHandler,
    MCM_IRQHandler,
    FTFL_IRQHandler,
    PMC_IRQHandler,
    LLW_IRQHandler,
    I2C0_IRQHandler,
    I2C1_IRQHandler,
    SPI0_IRQHandler,
    SPI1_IRQHandler,
    UART0_IRQHandler,
    UART1_IRQHandler,
    UART2_IRQHandler,
    ADC0_IRQHandler,
    CMP0_IRQHandler,
    FTM0_IRQHandler,
    FTM1_IRQHandler,
    FTM2_IRQHandler,
    RTC_Alarm_IRQHandler,
    RTC_Seconds_IRQHandler,
    PIT_IRQHandler,
    Fault_Handler,
    USBOTG_IRQHandler,
    DAC0_IRQHandler,
    TSI0_IRQHandler,
    MCG_IRQHandler,
    LPTMR0_IRQHandler,
    Fault_Handler,
    PORTA_IRQHandler,
    PORTD_IRQHandler
};
#endif /* MCU_MKL25Z4 */


#ifdef MCU_MKL43Z4
void (* const InterruptVector[])() __attribute__ ((section(".vectortable"))) = {
    (void(*)(void)) __SP_INIT,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    SVC_Handler,
    Fault_Handler,
    Fault_Handler,
    PendSV_Handler,
    SysTick_Handler,

    DMA0_IRQHandler,
    DMA1_IRQHandler,
    DMA2_IRQHandler,
    DMA3_IRQHandler,
    FTFA_IRQHandler,
    PMC_IRQHandler,
    LLW_IRQHandler,
    I2C0_IRQHandler,
    I2C1_IRQHandler,
    SPI0_IRQHandler,
    SPI1_IRQHandler,
    LPUART0_IRQHandler,
    LPUART1_IRQHandler,
    UART2_FLEXIO_IRQHandler,
    ADC0_IRQHandler,
    CMP0_IRQHandler,
    TPM0_IRQHandler,
    TPM1_IRQHandler,
    TPM2_IRQHandler,
    RTC_IRQHandler,
    RTC_Seconds_IRQHandler,
    PIT_IRQHandler,
    I2S0_IRQHandler,
    USB0_IRQHandler,
    DAC0_IRQHandler,
    LPTMR0_IRQHandler,
    LCD_IRQHandler,
    PORTA_IRQHandler,
    PORTCD_IRQHandler
};
#endif /* MCU_MKL43Z4 */


#ifdef MCU_MK22F25612
void (* const InterruptVector[])() __attribute__ ((section(".vectortable"))) = {
    (void(*)(void)) __SP_INIT,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    SVC_Handler,
    DebugMon_Handler,
    Fault_Handler,
    PendSV_Handler,
    SysTick_Handler,

    DMA0_IRQHandler,
    DMA1_IRQHandler,
    DMA2_IRQHandler,
    DMA3_IRQHandler,
    DMA4_IRQHandler,
    DMA5_IRQHandler,
    DMA6_IRQHandler,
    DMA7_IRQHandler,
    DMA8_IRQHandler,
    DMA9_IRQHandler,
    DMA10_IRQHandler,
    DMA11_IRQHandler,
    DMA12_IRQHandler,
    DMA13_IRQHandler,
    DMA14_IRQHandler,
    DMA15_IRQHandler,
    DMA_Error_IRQHandler,
    MCM_IRQHandler,
    FTF_IRQHandler,
    Read_Collision_IRQHandler,
    LVD_LVW_IRQHandler,
    LLW_IRQHandler,
    Watchdog_IRQHandler,
    RNG_IRQHandler,
    I2C0_IRQHandler,
    I2C1_IRQHandler,
    SPI0_IRQHandler,
    SPI1_IRQHandler,
    I2S0_Tx_IRQHandler,
    I2S0_Rx_IRQHandler,
    LPUART0_IRQHandler,
    UART0_RX_TX_IRQHandler,
    UART0_ERR_IRQHandler,
    UART1_RX_TX_IRQHandler,
    UART1_ERR_IRQHandler,
    UART2_RX_TX_IRQHandler,
    UART2_ERR_IRQHandler,
    Fault_Handler,
    Fault_Handler,
    ADC0_IRQHandler,
    CMP0_IRQHandler,
    CMP1_IRQHandler,
    FTM0_IRQHandler,
    FTM1_IRQHandler,
    FTM2_IRQHandler,
    Fault_Handler,
    RTC_IRQHandler,
    RTC_Seconds_IRQHandler,
    PIT0_IRQHandler,
    PIT1_IRQHandler,
    PIT2_IRQHandler,
    PIT3_IRQHandler,
    PDB0_IRQHandler,
    USB0_IRQHandler,
    Fault_Handler,
    Fault_Handler,
    DAC0_IRQHandler,
    MCG_IRQHandler,
    LPTMR0_IRQHandler,
    PORTA_IRQHandler,
    PORTB_IRQHandler,
    PORTC_IRQHandler,
    PORTD_IRQHandler,
    PORTE_IRQHandler,
    SWI_IRQHandler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    Fault_Handler,
    ADC1_IRQHandler
};
#endif /* MCU_MK22F25612 */

