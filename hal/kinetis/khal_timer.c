/**
 *
 *  Hardware Abstraction Layer for Kinetis LPTimer and SysTick modules.
 *
 *  COPYRIGHT NOTICE: (c) 2016 DDPA LLC
 *  All Rights Reserved
 *
 */


#include  <stdbool.h>
#include  <stdint.h>
#include  "contract.h"
#include  "derivative.h"
#include  "khal_clk.h"
#include  "khal_timer.h"


// ==== SYSTICK API ====

void  khal_timer_SysTickInit(uint32_t const reload_value, bool const  div_16, bool const interrupt, bool const run) {
    uint32_t  ctrl_val = (((div_16) ? 0 : 1) << 2) | (((interrupt) ? 1 : 0) << 1) | ((run) ? 1 : 0);

    SysTick->LOAD = reload_value - 1;     // counter includes zero, thus the -1
    SysTick->VAL  = 0;
    SysTick->CTRL = ctrl_val;             // RSVD      = 0x000 000
                                          // COUNTFLAG = 0
                                          // RSVD      = 0x000 0
                                          // CLKSOURCE = div_16 - 1
                                          // TICKINT   = interrupt
                                          // ENABLE    = run
}

/// Configure timer in free running mode with prescaler bypassed.
void  khal_timer_LPTimerInit(LPTMR_MemMapPtr const lptimer, khal_timer_lptimer_clk_src_t const clk_src, uint16_t const compare_value, bool const run) {
    khal_clk_ModuleClkGateEn(&SIM_SCGC5, SIM_SCGC5_LPTMR_SHIFT, true);    // enable LP Timer module
    lptimer->CMR = compare_value;
    lptimer->PSR = LPTMR_PSR_PCS(clk_src) | LPTMR_PSR_PBYP_MASK;


    lptimer->CSR = ((run) ? 1 : 0);   // start or stop the timer
    if (run) {                        // if starting, disable then re-enable in order to clear the counter
        lptimer->CSR &= ~LPTMR_CSR_TEN_MASK;
        lptimer->CSR |=  LPTMR_CSR_TEN_MASK;
    }
}

#if (defined MCU_MK22F25612)
    #define LPTMR0_IRQn   LPTimer_IRQn
#endif
void khal_timer_LPTimerIntEn(LPTMR_MemMapPtr const lptimer, uint32_t const priority, bool const enable) {

    /// TODO: Support more than LPTIMER0
    if (enable) {
        NVIC_ClearPendingIRQ(LPTMR0_IRQn);
        NVIC_SetPriority(LPTMR0_IRQn, priority);
        NVIC_EnableIRQ(LPTMR0_IRQn);
    }
    else {
        NVIC_DisableIRQ(LPTMR0_IRQn);
    }

    lptimer->CSR  = (lptimer->CSR & ~LPTMR_CSR_TIE_MASK) | (((enable) ? 1 : 0) << LPTMR_CSR_TIE_SHIFT);
}

uint16_t  khal_timer_LPTimerReadCounter(LPTMR_MemMapPtr const lptimer) {
    lptimer->CNR = 0;       // must write first to save the counter to a temp register
    return (lptimer->CNR);
}



