/*
 *    (c) Copyright 2018 DDPA LLC
 *    ALL RIGHTS RESERVED.
 *
 *    Startup code for M0/M4 processors.
 *
 *    Entry point for SAMD programs. This module is completely
 *    stand-alone and does not require any libraries.
 *
 *
 */

#include "sam.h"
#include "fault.h"

#define UNUSED_STACK_FILL  (0x55)
#undef  EIR_SEGMENT     // define if there is an Execute-In-RAM segment

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
 * Redirect here for all uninitialized vectors and assertions.
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

#ifdef  EIR_SEGMENT
    /*
     *  Copy code from read only memory to execute-in-RAM. Symbols must be aligned to 4 byte boundary.
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
#endif  /* EIR_SEGMENT */


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


#ifdef _SAMD51J19A_
/*
 * Weak definitions of handlers point to Fault_Handler if not implemented
 */
void NonMaskableInt_Handler      ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void HardFault_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void MemManagement_Handler       ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void BusFault_Handler            ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void UsageFault_Handler          ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SVCall_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void DebugMonitor_Handler        ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void PendSV_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SysTick_Handler             ( void ) __attribute__ ((weak, alias("Fault_Handler")));

void PM_Handler                  ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void MCLK_Handler                ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void OSCCTRL_0_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void OSCCTRL_1_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void OSCCTRL_2_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void OSCCTRL_3_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void OSCCTRL_4_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void OSC32KCTRL_Handler          ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SUPC_0_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SUPC_1_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void WDT_Handler                 ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void RTC_Handler                 ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EIC_0_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EIC_1_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EIC_2_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EIC_3_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EIC_4_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EIC_5_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EIC_6_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EIC_7_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EIC_8_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EIC_9_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EIC_10_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EIC_11_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EIC_12_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EIC_13_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EIC_14_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EIC_15_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void FREQM_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void NVMCTRL_0_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void NVMCTRL_1_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void DMAC_0_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void DMAC_1_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void DMAC_2_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void DMAC_3_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void DMAC_4_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EVSYS_0_Handler             ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EVSYS_1_Handler             ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EVSYS_2_Handler             ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EVSYS_3_Handler             ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EVSYS_4_Handler             ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void PAC_Handler                 ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void RAMECC_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM0_0_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM0_1_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM0_2_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM0_3_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM1_0_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM1_1_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM1_2_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM1_3_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM2_0_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM2_1_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM2_2_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM2_3_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM3_0_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM3_1_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM3_2_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM3_3_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM4_0_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM4_1_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM4_2_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM4_3_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM5_0_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM5_1_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM5_2_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM5_3_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void USB_0_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void USB_1_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void USB_2_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void USB_3_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC0_0_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC0_1_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC0_2_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC0_3_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC0_4_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC0_5_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC0_6_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC1_0_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC1_1_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC1_2_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC1_3_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC1_4_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC2_0_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC2_1_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC2_2_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC2_3_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC3_0_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC3_1_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC3_2_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC4_0_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC4_1_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC4_2_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TC0_Handler                 ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TC1_Handler                 ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TC2_Handler                 ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TC3_Handler                 ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TC4_Handler                 ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TC5_Handler                 ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void PDEC_0_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void PDEC_1_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void PDEC_2_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void ADC0_0_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void ADC0_1_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void ADC1_0_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void ADC1_1_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void AC_Handler                  ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void DAC_0_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void DAC_1_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void DAC_2_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void DAC_3_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void DAC_4_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void I2S_Handler                 ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void PCC_Handler                 ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void AES_Handler                 ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TRNG_Handler                ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void ICM_Handler                 ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void PUKCC_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void QSPI_Handler                ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SDHC0_Handler               ( void ) __attribute__ ((weak, alias("Fault_Handler")));


void (* const InterruptVector[])() __attribute__ ((section(".vectortable"))) = {
    (void(*)(void)) __SP_INIT,
    Reset_Handler,
    NonMaskableInt_Handler,
    HardFault_Handler,
    MemManagement_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0,
    0,
    0,
    0,
    SVCall_Handler,
    DebugMonitor_Handler,
    0,
    PendSV_Handler,
    SysTick_Handler,

    PM_Handler,
    MCLK_Handler,
    OSCCTRL_0_Handler,
    OSCCTRL_1_Handler,
    OSCCTRL_2_Handler,
    OSCCTRL_3_Handler,
    OSCCTRL_4_Handler,
    OSC32KCTRL_Handler,
    SUPC_0_Handler,
    SUPC_1_Handler,
    WDT_Handler,
    RTC_Handler,
    EIC_0_Handler,
    EIC_1_Handler,
    EIC_2_Handler,
    EIC_3_Handler,
    EIC_4_Handler,
    EIC_5_Handler,
    EIC_6_Handler,
    EIC_7_Handler,
    EIC_8_Handler,
    EIC_9_Handler,
    EIC_10_Handler,
    EIC_11_Handler,
    EIC_12_Handler,
    EIC_13_Handler,
    EIC_14_Handler,
    EIC_15_Handler,
    FREQM_Handler,
    NVMCTRL_0_Handler,
    NVMCTRL_1_Handler,
    DMAC_0_Handler,
    DMAC_1_Handler,
    DMAC_2_Handler,
    DMAC_3_Handler,
    DMAC_4_Handler,
    EVSYS_0_Handler,
    EVSYS_1_Handler,
    EVSYS_2_Handler,
    EVSYS_3_Handler,
    EVSYS_4_Handler,
    PAC_Handler,
    RAMECC_Handler,
    SERCOM0_0_Handler,
    SERCOM0_1_Handler,
    SERCOM0_2_Handler,
    SERCOM0_3_Handler,
    SERCOM1_0_Handler,
    SERCOM1_1_Handler,
    SERCOM1_2_Handler,
    SERCOM1_3_Handler,
    SERCOM2_0_Handler,
    SERCOM2_1_Handler,
    SERCOM2_2_Handler,
    SERCOM2_3_Handler,
    SERCOM3_0_Handler,
    SERCOM3_1_Handler,
    SERCOM3_2_Handler,
    SERCOM3_3_Handler,
    SERCOM4_0_Handler,
    SERCOM4_1_Handler,
    SERCOM4_2_Handler,
    SERCOM4_3_Handler,
    SERCOM5_0_Handler,
    SERCOM5_1_Handler,
    SERCOM5_2_Handler,
    SERCOM5_3_Handler,
    USB_0_Handler,
    USB_1_Handler,
    USB_2_Handler,
    USB_3_Handler,
    TCC0_0_Handler,
    TCC0_1_Handler,
    TCC0_2_Handler,
    TCC0_3_Handler,
    TCC0_4_Handler,
    TCC0_5_Handler,
    TCC0_6_Handler,
    TCC1_0_Handler,
    TCC1_1_Handler,
    TCC1_2_Handler,
    TCC1_3_Handler,
    TCC1_4_Handler,
    TCC2_0_Handler,
    TCC2_1_Handler,
    TCC2_2_Handler,
    TCC2_3_Handler,
    TCC3_0_Handler,
    TCC3_1_Handler,
    TCC3_2_Handler,
    TCC4_0_Handler,
    TCC4_1_Handler,
    TCC4_2_Handler,
    TC0_Handler,
    TC1_Handler,
    TC2_Handler,
    TC3_Handler,
    TC4_Handler,
    TC5_Handler,
    PDEC_0_Handler,
    PDEC_1_Handler,
    PDEC_2_Handler,
    ADC0_0_Handler,
    ADC0_1_Handler,
    ADC1_0_Handler,
    ADC1_1_Handler,
    AC_Handler,
    DAC_0_Handler,
    DAC_1_Handler,
    DAC_2_Handler,
    DAC_3_Handler,
    DAC_4_Handler,
    I2S_Handler,
    PCC_Handler,
    AES_Handler,
    TRNG_Handler,
    ICM_Handler,
    PUKCC_Handler,
    QSPI_Handler,
    SDHC0_Handler
};
#endif /* _SAMD51J19A_ */


#ifdef _SAMD11D14AS_
/*
 * Weak definitions of handlers point to Fault_Handler if not implemented
 */
void NMI_Handler             ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void HardFault_Handler       ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SVC_Handler             ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void PendSV_Handler          ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SysTick_Handler         ( void ) __attribute__ ((weak, alias("Fault_Handler")));

void PM_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SYSCTRL_Handler         ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void WDT_Handler             ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void RTC_Handler             ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EIC_Handler             ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void NVMCTRL_Handler         ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void DMAC_Handler            ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void USB_Handler             ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void EVSYS_Handler           ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM0_Handler         ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM1_Handler         ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void SERCOM2_Handler         ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TCC0_Handler            ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TC1_Handler             ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void TC2_Handler             ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void ADC_Handler             ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void AC_Handler              ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void DAC_Handler             ( void ) __attribute__ ((weak, alias("Fault_Handler")));
void PTC_Handler             ( void ) __attribute__ ((weak, alias("Fault_Handler")));


void (* const InterruptVector[])() __attribute__ ((section(".vectortable"))) = {
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

    PM_Handler,
    SYSCTRL_Handler,
    WDT_Handler,
    RTC_Handler,
    EIC_Handler,
    NVMCTRL_Handler,
    DMAC_Handler,
    USB_Handler,
    EVSYS_Handler,
    SERCOM0_Handler,
    SERCOM1_Handler,
    SERCOM2_Handler,
    TCC0_Handler,
    TC1_Handler,
    TC2_Handler,
    ADC_Handler,
    AC_Handler,
    DAC_Handler,
    PTC_Handler
};
#endif /* _SAMD11D14AS_ */

