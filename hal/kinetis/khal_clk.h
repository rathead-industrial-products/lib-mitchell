/**
 *
 *  @file  khal_clk.h
 *  @brief Hardware Abstraction Layer for Kinetis MCU clock and oscillator modules.
 *
 *  COPYRIGHT NOTICE: (c) 2016 DDPA LLC
 *  All Rights Reserved
 *
 */


#ifndef _khal_clk_H_
#define _khal_clk_H_

#include  <stdbool.h>
#include  <stdint.h>
#include  "derivative.h"

// ==== Typedefs ====

typedef enum khal_clk_mcgl_clks_t {
    KHAL_CLK_CLKS_HIRC = 0,
    KHAL_CLK_CLKS_LIRC = 1,
    KHAL_CLK_CLKS_EXT  = 2,
    KHAL_CLK_CLKS_ERR
} khal_clk_mcgl_clks_t;

typedef enum khal_clk_clkout_sel_t {
    KHAL_CLK_CLKOUT_SEL_BUSCLK    = 2,
    KHAL_CLK_CLKOUT_SEL_LPO       = 3,
    KHAL_CLK_CLKOUT_SEL_LIRC      = 4,
    KHAL_CLK_CLKOUT_SEL_OSCERCLK  = 6,
    KHAL_CLK_CLKOUT_SEL_IRC48M    = 7,
    KHAL_CLK_CLKOUT_SEL_DIS       = 8
} khal_clk_clkout_sel_t;

typedef enum khal_clk_clkout32K_sel_t {
    KHAL_CLK_CLKOUT32K_SEL_OSC32KCLK  = 0,
    KHAL_CLK_CLKOUT32K_SEL_RTC_CLKIN  = 2,
    KHAL_CLK_CLKOUT32K_SEL_LPO        = 3,
    KHAL_CLK_CLKOUT32K_SEL_DIS        = 4
} khal_clk_clkout32K_sel_t;


// ==== SIM MODULE CLOCK GATING API ====

/// Enable the clock to the selected module.
void khal_clk_ModuleClkGateEn(volatile uint32_t * scg_reg, uint32_t bit, bool enable);


// ==== CLOCK OUT CONTROL API ====

/// Drive the selected clock output on the specified port/pin.
void khal_clk_ClkoutSel(PORT_MemMapPtr clkout_port, uint32_t clkout_pin, uint32_t clkout_mux, khal_clk_clkout_sel_t clkout_sel);

/// Drive the selected ERCLK32K clock output on the specified port/pin.
void khal_clk_Clkout32KSel(PORT_MemMapPtr clkout32k_port, uint32_t clkout32k_pin, uint32_t clkout32k_mux, khal_clk_clkout32K_sel_t clkout32k_sel);


// ==== CLOCKS AND DIVIDERS API ====

/// Initialize the MCG-Lite.
void khal_clk_Init(void);

/// Enable (true) or disable (false) the Low-frequency Internal Reference Clock.
void khal_clk_SetLIRCEn(bool const enable);

/// Enable (true) or disable (false) the Low-frequency Internal Reference Clock in STOP mode.
void khal_clk_SetLIRCStopEn(bool const enable);

/// Set the Low-frequency Internal Reference Clock Select to 2 MHz (ircs = 0) or 8 MHz (icrs = 1).
void khal_clk_SetLIRCSelMode(uint32_t const ircs);

/// Set the Low-frequency Internal Reference Clock Divider.
void khal_clk_SetLIRCRefDiv(uint32_t const frcdiv);

/// Set the Second Low-frequency Internal Reference Clock Divider.
void khal_clk_SetLIRCRefDiv2(uint32_t const lirc_div2);

/// Enable (true) or disable (false) the High-frequency Internal Reference Clock source.
void khal_clk_SetHIRCEn(bool const enable);

/// Select the clock source for MCGOUTCLK.
void khal_clk_SetMCGOUTCLKSource(khal_clk_mcgl_clks_t const clks);

/// Return the clock source for MCGOUTCLK.
khal_clk_mcgl_clks_t khal_clk_GetMCGOUTCLKSource(void);

/// Set the core clock divider value.
khal_clk_mcgl_clks_t khal_clk_SetCoreDiv(uint32_t const outdiv1);

/// Set the bus clock divider value.
khal_clk_mcgl_clks_t khal_clk_SetBusDiv(uint32_t const outdiv4);




#endif  /* _khal_clk_H_ */


