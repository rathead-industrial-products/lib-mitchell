/**
 *
 *  samd_port.c
 *  Hardware Abstraction Layer for Microchip SAMD MCU PORT module.
 *
 *  COPYRIGHT NOTICE: (c) 2019 DDPA LLC
 *  All Rights Reserved
 *
 */


#include  <stdbool.h>
#include  <stdint.h>
#include  "sam.h"
#include  "samd_port.h"


static samd_port_int_callback_fn_t g_int_callback[8] = { 0 }; // total number of external interrupts


// ==== API ====

void samd_port_Init(samd_port_pin_t port_pin,
        uint32_t                    output_level,
        samd_port_oe_t              oe,
        samd_port_ie_t              ie,
        samd_port_ds_t              ds,
        samd_port_pull_mode_t       pm,
        samd_port_peripheral_mux_t  pin_func)
{
    samd_port_Disable(port_pin);
    SAMD_PORT_PIN_PUT(port_pin, output_level);
    if (oe) { SAMD_PORT_PIN_OUTPUT_ENABLE(port_pin); }
    if (ie) { SAMD_PORT_PIN_INPUT_ENABLE(port_pin); }
    if (ds) { SAMD_PORT_PIN_DRIVE_STRENGTH_HIGH(port_pin); }
    if (pm) { SAMD_PORT_PIN_PULL_ENABLE(port_pin); }
    samd_port_AltFunc(port_pin, pin_func);
}

#if defined(__SAMD21__)
void samd_port_IntConfig(samd_port_pin_t  port_pin,
             uint32_t                     extint,
             samd_port_int_t              sense,
             bool                         filter,
             samd_port_int_callback_fn_t  cb)
{
    static bool init;
    uint32_t    config = 0;

    if (!init) {
        // enable GCLK_EIC (30uS per filter sample, 3 samples per edge)
        GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_EIC) |  // EIC clock
                            GCLK_CLKCTRL_GEN_GCLK2               |  // Generic Clock Generator 2 is source OSCULP32K
                            GCLK_CLKCTRL_CLKEN ;

        NVIC_EnableIRQ(EIC_IRQn);
        NVIC_SetPriority(EIC_IRQn, 0x00); // highest priority
        EIC->CTRL.bit.ENABLE = 1;         // enable EIC
        init = true;
    }
    samd_port_AltFunc(port_pin, SAMD_PORT_PERIPHERAL_FUNC_A);
    g_int_callback[extint] = cb;

    if (filter) { config = (1 << EIC_CONFIG_FILTEN0_Pos); }
    config |= (uint32_t) sense;
    config <<= EIC_CONFIG_SENSE1_Pos * extint;
    EIC->CONFIG[0].reg &= ~(EIC_CONFIG_SENSE0_Msk << (EIC_CONFIG_SENSE1_Pos * extint));
    EIC->CONFIG[0].reg |= config;
    EIC->INTFLAG.reg  = (1 << extint);    // clear any existing interrupt
    EIC->INTENSET.reg = (1 << extint);    // enable interrupt
}
#endif

void samd_port_Disable(samd_port_pin_t port_pin) {
    SAMD_PORT_PIN_OUTPUT_DISABLE(port_pin);
    SAMD_PORT_PIN_INPUT_DISABLE(port_pin);
    SAMD_PORT_PIN_DRIVE_STRENGTH_NORMAL(port_pin);
    SAMD_PORT_PIN_PULL_DISABLE(port_pin);
    SAMD_PORT(port_pin).PINCFG[SAMD_PIN(port_pin)].bit.PMUXEN = 0;
}

void samd_port_AltFunc(samd_port_pin_t port_pin, samd_port_peripheral_mux_t func) {
    if (func == SAMD_PORT_PERIPHERAL_FUNC_GPIO) {
        SAMD_PORT(port_pin).PINCFG[SAMD_PIN(port_pin)].bit.PMUXEN = 0;
    }
    else {
        samd_port_Disable(port_pin);
        if (SAMD_PIN(port_pin) & 0x01)  { SAMD_PORT(port_pin).PMUX[SAMD_PIN(port_pin)/2].bit.PMUXO = func; }
        else                            { SAMD_PORT(port_pin).PMUX[SAMD_PIN(port_pin)/2].bit.PMUXE = func; }
        SAMD_PORT(port_pin).PINCFG[SAMD_PIN(port_pin)].bit.PMUXEN = 1;
    }
}

void EIC_Handler(void) {
    uint32_t extint = 0;
    uint32_t intflag;

    intflag = EIC->INTFLAG.reg;
    while (intflag >>= 1) { ++extint; }
    g_int_callback[extint]((void *) extint);

    EIC->INTFLAG.reg = (1 << extint);    // clear interrupt

}





