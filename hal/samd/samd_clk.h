/**
 *
 *  samd_clk.h
 *  Hardware Abstraction Layer for SAMD MCU clock and oscillator modules.
 *
 *  COPYRIGHT NOTICE: (c) 2018 DDPA LLC
 *  All Rights Reserved
 *
 */


#ifndef _samd_clk_H_
#define _samd_clk_H_

#include  <stdbool.h>
#include  <stdint.h>



// ==== CLOCK SYSTEM INITIALIZATION API ====

// SAMD09/10/11/21  GCLK_MAIN =  48MHz, GCLK1 = XOSC32K, GCLK2 = OSCULP32K, GCLK3 = OSC8M
// SAMD51           GCLK_MAIN = 120MHz, GCLK1 = 48 MHz,  GCLK2 = 100MHz,    GCLK3 = XOSC32K, GCLK8 = 12 MHz
void samd_clkInit(void);


// ==== CLOCK SOURCE API ====

// Reconfigure GCLK_MAIN after samd_clkInit() - SAMD09/10/11/21 Only
// Changes Systick reload value to maintain 1 ms ticks
void samd21_GCLK_MAIN_48MCL(void);      // locked to XOSC32K
void samd21_GCLK_MAIN_8M(void);         // 0SC8M
void samd21_GCLK_MAIN_1M(void);         // 0SC8M / 8
void samd21_GCLK_MAIN_32K(void);        // XOSC32K


// ==== EXTERNAL CLOCK OUT API ====

// Drive the specified gclk out the selected pin
// No other port configuration is required
void samd11_glck0OutPA24(bool enable);
void samd11_glck1OutPA22(bool enable);
void samd11_glck2OutPA16(bool enable);
void samd11_glck4OutPA14(bool enable);
void samd11_glck5OutPA15(bool enable);

void samd51_glck0OutPA14(bool enable);
void samd51_glck2OutPA16(bool enable);
void samd51_glck3OutPB17(bool enable);
void samd51_glck1OutPB23(bool enable);


#endif  /* _samd_clk_H_ */


