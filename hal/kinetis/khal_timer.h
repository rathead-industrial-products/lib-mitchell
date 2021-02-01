/**
 *
 *  @file  khal_timer.h
 *  @brief Hardware Abstraction Layer for Kinetis MCU LPTimer and SysTick modules.
 *
 *  COPYRIGHT NOTICE: (c) 2016 DDPA LLC
 *  All Rights Reserved
 *
 */


#ifndef _khal_timer_H_
#define _khal_timer_H_

#include  <stdbool.h>
#include  <stdint.h>
#include  "derivative.h"


// ==== Typedefs ====

typedef enum khal_timer_lptimer_clk_src_t {
    KHAL_TIMER_LPTIMER_CLK_SRC_MCGIRCLK = 0,
    KHAL_TIMER_LPTIMER_CLK_SRC_LPO      = 1,
    KHAL_TIMER_LPTIMER_CLK_SRC_ERCLK32K = 2,
    KHAL_TIMER_LPTIMER_CLK_SRC_OSCERCLK = 3
} khal_timer_lptimer_clk_src_t;


// ==== SYSTICK API ====

/// Initialize SysTick. Clock is the core clock, or core clock / 16 if div_16 is true.
/// An interrupt will be generated if interrupt is true, and SysTick will be started if run is true.
void  khal_timer_SysTickInit(uint32_t const reload_value, bool const div_16, bool const interrupt, bool const run);

/// Initialize the Low Power Timer.
/// Interrupts are disabled and the timer will be started if run is true.
void  khal_timer_LPTimerInit(LPTMR_MemMapPtr const lptimer, khal_timer_lptimer_clk_src_t const clk_src, uint16_t const compare_value, bool const run);

/// Enable (true) or disable (false) the LP Timer interrupt.
void khal_timer_LPTimerIntEn(LPTMR_MemMapPtr const lptimer, uint32_t const priority, bool const enable);

/// Return the Low Power Timer counter value.
uint16_t  khal_timer_LPTimerReadCounter(LPTMR_MemMapPtr const lptimer);



#endif  /* _khal_timer_H_ */


