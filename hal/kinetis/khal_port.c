/**
 *
 *  Hardware Abstraction Layer for Kinetis PORT & GPIO modules.
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


// ==== Defines ====

/* Number of ports, and macros that are undefined in the derivative file */
#if !(defined PORT_PCR_ODE_SHIFT)
#define PORT_PCR_ODE_SHIFT      5
#endif

#if (defined MCU_MKL03Z4)
#define KHAL_GPIO_PORTS         2   // A, B
#endif

#if (defined MCU_MK22F25612)
#define KHAL_GPIO_PORTS         5   // A, B, C, D, E
#endif

#if (defined MCU_MKL25Z4)
#define KHAL_GPIO_PORTS         5   // A, B, C, D, E
#endif


// ==== API ====

void _khal_port_GetPORTandGPIO(PORT_MemMapPtr const port, uint32_t const gpio_ports, GPIO_MemMapPtr *gpio, uint32_t *port_num) {
    *port_num = (((uint32_t) port) - PORTA_BASE) / (PORTB_BASE - PORTA_BASE);  // 0 = A, 1 = B, ...
    *gpio     = ((GPIO_MemMapPtr[]) GPIO_BASE_PTRS)[*port_num];

    REQUIRE (*port_num < gpio_ports);
}

/// Writes a zero to all locations not specified here.
void khal_port_Init(PORT_MemMapPtr port, uint32_t pin, uint32_t               mux,
                                                       khal_port_data_dir_t   dir,
                                                       uint32_t               output_level,
                                                       khal_port_attr_en_t    pe,
                                                       khal_port_attr_en_t    ps,
                                                       khal_port_attr_en_t    sre,
                                                       khal_port_attr_en_t    pfe,
                                                       khal_port_attr_en_t    ode,
                                                       khal_port_attr_en_t    dse,
                                                       khal_port_int_config_t irqc) {
    uint32_t        port_num;
    GPIO_MemMapPtr  gpio;

    _khal_port_GetPORTandGPIO(port, KHAL_GPIO_PORTS, &gpio, &port_num);

    REQUIRE (pin < 32);
    REQUIRE (mux < 8);
    REQUIRE (irqc < KHAL_PORT_INT_DMA_ERR);
    REQUIRE (dir < KHAL_PORT_DATA_DIR_ERR);
    REQUIRE (output_level <= 1);

    khal_clk_ModuleClkGateEn(&SIM_SCGC5, SIM_SCGC5_PORTA_SHIFT + port_num, true);    // enable PORT module

    port->PCR[pin] = (PORT_PCR_IRQC(irqc))        |
                     (PORT_PCR_MUX(mux))          |
                     (dse << PORT_PCR_DSE_SHIFT)  |
                     (ode << PORT_PCR_ODE_SHIFT)  |
                     (pfe << PORT_PCR_PFE_SHIFT)  |
                     (sre << PORT_PCR_SRE_SHIFT)  |
                     (pe  << PORT_PCR_PE_SHIFT)   |
                     (ps  << PORT_PCR_PS_SHIFT);

    gpio->PDOR = output_level << pin;
    gpio->PDDR = dir << pin;
}

void khal_port_Disable(PORT_MemMapPtr port, uint32_t pin) {
    uint32_t        port_num;
    GPIO_MemMapPtr  gpio;

    _khal_port_GetPORTandGPIO(port, KHAL_GPIO_PORTS, &gpio, &port_num);
    REQUIRE (pin < 32);

    port->PCR[pin] = port->PCR[pin] & ~PORT_PCR_MUX_MASK;
}






