/**
 *
 *  samd_port.c
 *  Hardware Abstraction Layer for Microchip SAMD MCU Real-Time Clock Module.
 *
 *  COPYRIGHT NOTICE: (c) 2019 DDPA LLC
 *  All Rights Reserved
 *
 */


#include  <stdbool.h>
#include  <stdint.h>
#include  <stddef.h>
#include  "sam.h"
#include  "samd_rtc.h"

#define SAMD_RTC_COUNT_TO_MS(count)     (((count * 1000) + 500) / 1024)
#define SAMD_RTC_MS_TO_COUNT(ms)        (((ms *    1024) + 500) / 1000)


/****************************** CLOCK FUNCTIONS *******************************
 *
 * The RTC counts ticks from XOSC32K. This provides a stable approx. millisecond
 * resolution clock in the absence of a stable SysTick, allowing the CPU clock
 * (which drives SysTick) to be varied or stopped.
 *
 ******************************************************************************/

volatile static uint32_t     _g_RTC_interrupt_interval = 0;    // for periodic interrupts
volatile static voidFuncPtr  _g_RTC_callBack = NULL;           // RTC interrupt user handler
volatile static bool         _g_f_playing_possum = false;      // flag set by ISR to release spin loop


/*******************************************************************************

    void samd_RTCInit(void)

    Set up the RTC to run continuously from XOSC32K in 32 bit counter mode
    with a divide-by-32 prescaler. The count register represents seconds
    in 22.10 fractional format.

    XOSC32K must be configured to be running and driving the generic clock
    generator and keep running in standby.

    A software reset of the RTC is done upon initialization.

******************************************************************************/
void samd_RTCInit(void) {


    #if   defined(__SAMD11__)

        // keep the XOSC32K running in standy
        SYSCTRL->XOSC32K.reg |= SYSCTRL_XOSC32K_RUNSTDBY;

        // attach GCLK_RTC to generic clock generator 1
        GCLK->CLKCTRL.reg = (uint32_t)((GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK1 | (RTC_GCLK_ID << GCLK_CLKCTRL_ID_Pos)));

        // ensure module is reset
        RTC->MODE0.CTRL.bit.SWRST = 1;
        while (RTC->MODE0.CTRL.bit.SWRST == 1);

        // reset configuration is mode=0, no clear on match
        RTC->MODE0.CTRL.reg = RTC_MODE0_CTRL_PRESCALER_DIV32 | RTC_MODE0_CTRL_ENABLE;


    #elif defined(__SAMD51__)

        // keep the XOSC32K running in standy
        OSC32KCTRL->XOSC32K.bit.RUNSTDBY = 1;

        // attach RTC to XOSC32K
        OSC32KCTRL->RTCCTRL.bit.RTCSEL = OSC32KCTRL_RTCCTRL_RTCSEL_XOSC32K_Val;

        // ensure module is reset
        RTC->MODE0.CTRLA.bit.ENABLE = 0;   // disable rtc
        while (RTC->MODE0.SYNCBUSY.reg != 0);
        RTC->MODE0.CTRLA.bit.SWRST = 1;
        while (RTC->MODE0.SYNCBUSY.reg != 0);

        // reset configuration is mode=0, no clear on match
        RTC->MODE0.CTRLA.reg = RTC_MODE0_CTRLA_COUNTSYNC | RTC_MODE0_CTRLA_PRESCALER_DIV32 | RTC_MODE0_CTRLA_MODE_COUNT32 | RTC_MODE0_CTRLA_ENABLE;
        while (RTC->MODE0.SYNCBUSY.reg != 0);

    #endif

    NVIC_EnableIRQ(RTC_IRQn);
    NVIC_SetPriority(RTC_IRQn, 0x00);

    samd_RTCSetClock(0); // reset to zero in case of warm start
}


/*******************************************************************************

    uint32_t samd_RTCSleep(void)

    Put the chip into standby mode. Return the time in milliseconds spent asleep.

******************************************************************************/
uint32_t samd_RTCSleep(void) {
    uint32_t enter, exit, et;

    enter = samd_RTCGetClock();

    #if   defined(__SAMD11__)
        SCB->SCR |=  SCB_SCR_SLEEPDEEP_Msk;

    #elif defined(__SAMD51__)
        while (!(PM->INTFLAG.bit.SLEEPRDY));   // wait for MAINVREG to be stable
        PM->SLEEPCFG.bit.SLEEPMODE = PM_SLEEPCFG_SLEEPMODE_STANDBY;
        PM->STDBYCFG.reg = 0;   // fast wakeup disabled, all RAM retained
    #endif

    __DSB();
    __WFE();

    exit = samd_RTCGetClock();
    et = exit - enter;
    et = SAMD_RTC_COUNT_TO_MS(et);
    return (et);
}


/*******************************************************************************

    void samd_RTCSleepFor(uint32_t ms)

    Put the chip into standby mode for ms milliseconds.
    Return the time in milliseconds spent asleep.
    Reconfigures the RTC alarm.

******************************************************************************/
uint32_t samd_RTCSleepFor(uint32_t ms) {
    uint32_t count = SAMD_RTC_MS_TO_COUNT(ms);

    if (count == 0) { return (0); }
    samd_RTCInterruptAt(count + samd_RTCGetClock(), NULL);
    return (samd_RTCSleep());
}


/*******************************************************************************

    uint32_t samd_RTCPlayPossum(void)

    Use in place of samd_RTCSleep for debugging. Spins on RTC interrupt bit.
    Cannot _WFE because the reason we are not actually sleeping is to
    let other processes continue to run that require clocks, like USB.
    Return the time in milliseconds spent "asleep".

******************************************************************************/
uint32_t samd_RTCPlayPossum(void) {
    uint32_t enter, exit, et;

    enter = samd_RTCGetClock();

    _g_f_playing_possum = true;
    while (_g_f_playing_possum);

    exit = samd_RTCGetClock();
    et = exit - enter;
    et = SAMD_RTC_COUNT_TO_MS(et);
    return (et);
}


/*******************************************************************************

    uint32_t samd_RTCPlayPossumFor(uint32_t ms)

    Use in place of samd_RTCSleepFor for debugging. Spins on RTC interrupt bit.
    Cannot _WFE because the reason we are not actually sleeping is to
    let other processes continue to run that require clocks, like USB.
    Return the time in milliseconds spent "asleep".
    Reconfigures the RTC alarm.

******************************************************************************/
uint32_t samd_RTCPlayPossumFor(uint32_t ms) {
    uint32_t count = SAMD_RTC_MS_TO_COUNT(ms);

    if (count == 0) { return (0); }
    samd_RTCInterruptAt(count + samd_RTCGetClock(), NULL);
    return(samd_RTCPlayPossum());
}


/*******************************************************************************

    uint32_t samd_RTCGetClock(void)

    Return the RTC counter value.

******************************************************************************/
uint32_t samd_RTCGetClock(void) {
    uint32_t  count = RTC->MODE0.COUNT.reg;
    #if defined(__SAMD51__)
        while (RTC->MODE0.SYNCBUSY.reg != 0);
    #endif
    return (count);
}


/*******************************************************************************

    void samd_RTCSetClock(const uint32_t count)

    Set the value of the RTC counter to count. This may cause an interrupt
    to be missed.

******************************************************************************/
void samd_RTCSetClock(const uint32_t count) {
    RTC->MODE0.COUNT.reg = count;
    #if defined(__SAMD51__)
        while (RTC->MODE0.SYNCBUSY.reg != 0);
    #endif
}


/*******************************************************************************

    void samd_RTCDelay(const uint32_t ms)

    Wait in a blocking loop for ms milliseconds.

******************************************************************************/
void samd_RTCDelay(const uint32_t ms) {
    uint32_t count = SAMD_RTC_MS_TO_COUNT(ms);
    uint32_t start = samd_RTCGetClock();
    while (samd_RTCGetClock() < (start + count));
}


/*******************************************************************************

    void samd_RTCInterruptEvery(const uint32_t ms, const voidFuncPtr callback)

    Generate an interrupt every ms milliseconds.

******************************************************************************/
void samd_RTCInterruptEvery(const uint32_t ms, const voidFuncPtr callback) {
    uint32_t count = SAMD_RTC_MS_TO_COUNT(ms);

    if (count == 0) { return; }   // zero interval illegal

    samd_RTCInterruptDisable();
    _g_RTC_interrupt_interval = count;
    _g_RTC_callBack = callback;

    // clear any pending interrupts, set compare register and enable interrupt
    RTC->MODE0.INTFLAG.reg = RTC_MODE0_INTFLAG_MASK;
    RTC->MODE0.COMP[0].reg = samd_RTCGetClock() + count;
    RTC->MODE0.INTENSET.bit.CMP0 = 1;
}


/*******************************************************************************

    void samd_RTCInterruptAt(const count ms, const voidFuncPtr callback)

    Generate an interrupt at RTC time ms milliseconds.

******************************************************************************/
void samd_RTCInterruptAt(const uint32_t ms, const voidFuncPtr callback) {
    uint32_t count = SAMD_RTC_MS_TO_COUNT(ms);

    if ((samd_RTCGetClock() + count) < 1) { return; }   // zero interval illegal

    samd_RTCInterruptDisable();
    _g_RTC_callBack = callback;

    // clear any pending interrupts, set compare register and enable interrupt
    RTC->MODE0.INTFLAG.reg = RTC_MODE0_INTFLAG_MASK;
    RTC->MODE0.COMP[0].reg = count;
    RTC->MODE0.INTENSET.bit.CMP0 = 1;
}


/*******************************************************************************

    void samd_RTCInterruptDisable(void)

    Disable all RTC interrupts.

******************************************************************************/
void samd_RTCInterruptDisable(void) {
    _g_RTC_interrupt_interval = 0;
    RTC->MODE0.INTENCLR.reg = RTC_MODE0_INTENCLR_MASK;
}


/*******************************************************************************

    void RTC_Handler(void)

    RTC interrupt vector points here. If there are periodic interrupts,
    reset the match register. If a one-time interrupt disable interrupts
    to prevent another interrupt on timer rollover.

******************************************************************************/
void RTC_Handler(void)
{
    RTC->MODE0.INTFLAG.reg = RTC_MODE0_INTFLAG_MASK; // clear all interrupt sources

    if (_g_RTC_interrupt_interval != 0) {
        RTC->MODE0.COMP[0].reg = RTC->MODE0.COMP[0].reg + _g_RTC_interrupt_interval;
    }

    if (_g_RTC_interrupt_interval == 0) {
        samd_RTCInterruptDisable();
    }

    /*
     * Interrupts cannot be enabled without calling a function that sets
     * RTC_callback so there will never be a stale callback if interrupts
     * are enabled.
     *
     * Putting the callback at the end of the handler allows the callback
     * to set a new or different interrupt.
     */
    if (_g_RTC_callBack != NULL) { _g_RTC_callBack(); }

    _g_f_playing_possum = false;    // release fake sleep from spin loop
}

