/**
 *
 *  Hardware Abstraction Layer for Kinetis MCU clock and oscillator modules.
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
#include  "khal_port.h"


// ==== SIM MODULE CLOCK GATING API ====

void khal_clk_ModuleClkGateEn(volatile uint32_t * scg_reg, uint32_t bit, bool enable) {
    uint32_t bit_mask = 1 << bit;
    *scg_reg = (*scg_reg & ~bit_mask) | ((enable) ? 1 : 0) << bit;
}


// ==== CLOCK OUT CONTROL API ====

void khal_clk_ClkoutSel(PORT_MemMapPtr clkout_port, uint32_t clkout_pin, uint32_t clkout_mux, khal_clk_clkout_sel_t clkout_sel) {
    if (clkout_sel == KHAL_CLK_CLKOUT_SEL_DIS) { clkout_mux = 0; }    // disable pin
    khal_port_Init(clkout_port, clkout_pin, clkout_mux,
                   KHAL_PORT_DATA_DIR_IN, 0,
                   KHAL_PORT_ATTR_PE_DIS, KHAL_PORT_ATTR_PS_UP,
                   KHAL_PORT_ATTR_SRE_FAST, KHAL_PORT_ATTR_PFE_DIS,
                   KHAL_PORT_ATTR_ODE_DIS, KHAL_PORT_ATTR_DSE_LOW,
                   KHAL_PORT_INT_DMA_DIS);
    SIM_SOPT2 = (SIM_SOPT2 & ~SIM_SOPT2_CLKOUTSEL_MASK) | SIM_SOPT2_CLKOUTSEL(clkout_sel);
}

void khal_clk_Clkout32KSel(PORT_MemMapPtr clkout32k_port, uint32_t clkout32k_pin, uint32_t clkout32k_mux, khal_clk_clkout32K_sel_t clkout32k_sel) {
    if (clkout32k_sel == KHAL_CLK_CLKOUT32K_SEL_DIS) {
        #if !(defined MCU_MKL25Z4)  // KL25 does not have OSC32KOUT
            SIM_SOPT1 = (SIM_SOPT1 & ~SIM_SOPT1_OSC32KOUT_MASK) | SIM_SOPT1_OSC32KOUT(0);
        #endif
        clkout32k_mux = 0; // disable pin
    }
    else {
        #if !(defined MCU_MKL25Z4)  // KL25 does not have OSC32KOUT
            SIM_SOPT1 = (SIM_SOPT1 & ~SIM_SOPT1_OSC32KOUT_MASK) | SIM_SOPT1_OSC32KOUT(1);
        #endif
    }
    khal_port_Init(clkout32k_port, clkout32k_pin, clkout32k_mux,
                   KHAL_PORT_DATA_DIR_IN, 0,
                   KHAL_PORT_ATTR_PE_DIS, KHAL_PORT_ATTR_PS_UP,
                   KHAL_PORT_ATTR_SRE_SLOW, KHAL_PORT_ATTR_PFE_DIS,
                   KHAL_PORT_ATTR_ODE_DIS, KHAL_PORT_ATTR_DSE_LOW,
                   KHAL_PORT_INT_DMA_DIS);
    SIM_SOPT1 = (SIM_SOPT1 & ~SIM_SOPT1_OSC32KSEL_MASK) | SIM_SOPT1_OSC32KSEL(clkout32k_sel);
}


// ==== CLOCKS AND DIVIDERS API ====

void khal_clk_Init(void) {
    /* 48 MHz core/system clock, 24 MHz bus/flash clock */
    khal_clk_SetHIRCEn(true);                                     // enable 48 MHz source
    khal_clk_SetBusDiv(1);                                        // divide by 2
    khal_clk_SetCoreDiv(0);                                       // divide by 1
    khal_clk_SetMCGOUTCLKSource(KHAL_CLK_CLKS_HIRC);              // select 48 MHz source
    while (khal_clk_GetMCGOUTCLKSource() != KHAL_CLK_CLKS_HIRC);  // spin till clk stable
}

void khal_clk_SetLIRCEn(bool const enable) {
    MCG_C1 = (MCG_C1 & ~MCG_C1_IRCLKEN_MASK) | ((enable ? 1 : 0) << MCG_C1_IRCLKEN_SHIFT);
}

void khal_clk_SetLIRCStopEn(bool const enable) {
    MCG_C1 = (MCG_C1 & ~MCG_C1_IREFSTEN_MASK) | (enable ? 1 : 0);
}

void khal_clk_SetLIRCSelMode(uint32_t const ircs) {
    REQUIRE (ircs <= 1);
    MCG_C2 = (MCG_C2 & ~MCG_C2_IRCS_MASK) | ircs;
}

void khal_clk_SetLIRCRefDiv(uint32_t const frcdiv) {
    REQUIRE (frcdiv <= 7);
    MCG_SC = MCG_SC_FCRDIV(frcdiv);
}

void khal_clk_SetLIRCRefDiv2(uint32_t const lirc_div2) {
    REQUIRE (lirc_div2 <= 7);
    #if defined (MCU_MKL03Z4) || defined (MCU_MKL43Z4)
        MCG_MC = (MCG_MC & ~MCG_MC_LIRC_DIV2_MASK) | MCG_MC_LIRC_DIV2(lirc_div2);
    #else
        #warning  "khal_clk_SetLIRCRefDiv2 not supported in this processor."
    #endif
    // else no-op
}

void khal_clk_SetHIRCEn(bool const enable) {
    #if (defined MCU_MKL03Z4) || (defined MCU_MKL43Z4)
        MCG_MC = (MCG_MC & ~MCG_MC_HIRCEN_MASK) | ((enable ? 1 : 0) << MCG_MC_HIRCEN_SHIFT);
    #else
        #warning  "khal_clk_SetHIRCEn not supported in this processor."
    #endif
    // else no-op
}

void khal_clk_SetMCGOUTCLKSource(khal_clk_mcgl_clks_t const clks) {
    REQUIRE (clks < KHAL_CLK_CLKS_ERR);
    MCG_C1 = (MCG_C1 & ~MCG_C1_CLKS_MASK) | MCG_C1_CLKS(clks);
}

khal_clk_mcgl_clks_t khal_clk_GetMCGOUTCLKSource(void) {
    return (khal_clk_mcgl_clks_t) ((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT);
}

khal_clk_mcgl_clks_t khal_clk_SetCoreDiv(uint32_t const outdiv1) {
    REQUIRE (outdiv1 <= 15);
    SIM_CLKDIV1 = (SIM_CLKDIV1 & ~SIM_CLKDIV1_OUTDIV1_MASK) | SIM_CLKDIV1_OUTDIV1(outdiv1);
}

khal_clk_mcgl_clks_t khal_clk_SetBusDiv(uint32_t const outdiv4) {
    REQUIRE (outdiv4 <= 15);
    SIM_CLKDIV1 = (SIM_CLKDIV1 & ~SIM_CLKDIV1_OUTDIV4_MASK) | SIM_CLKDIV1_OUTDIV4(outdiv4);
}




