/**
 *
 *  @file  khal_port.h
 *  @brief Hardware Abstraction Layer for Kinetis MCU PORT & GPIO modules.
 *
 *  COPYRIGHT NOTICE: (c) 2016 DDPA LLC
 *  All Rights Reserved
 *
 */


#ifndef _khal_port_H_
#define _khal_port_H_

#include  <stdbool.h>
#include  <stdint.h>
#include  "derivative.h"


// ==== Typedefs ====

/// Data Direction named constants.
typedef enum khal_port_data_dir_t {
    KHAL_PORT_DATA_DIR_IN   = 0,
    KHAL_PORT_DATA_DIR_OUT  = 1,
    KHAL_PORT_DATA_DIR_ERR  = 2
} khal_port_data_dir_t;

/// PORT Control Register configuration enables.
typedef enum khal_port_attr_en_t {
    KHAL_PORT_ATTR_PS_DN    = 0,        ///< pull select - pull up or pull down
    KHAL_PORT_ATTR_PS_UP    = 1,
    KHAL_PORT_ATTR_PE_DIS   = 0,        ///< pull enable
    KHAL_PORT_ATTR_PE_EN    = 1,
    KHAL_PORT_ATTR_SRE_FAST = 0,        ///< slew rate - fast or slow
    KHAL_PORT_ATTR_SRE_SLOW = 1,
    KHAL_PORT_ATTR_PFE_DIS  = 0,        ///< passive filter
    KHAL_PORT_ATTR_PFE_EN   = 1,
    KHAL_PORT_ATTR_ODE_DIS  = 0,        ///< open drain
    KHAL_PORT_ATTR_ODE_EN   = 1,
    KHAL_PORT_ATTR_DSE_LOW  = 0,        ///< drive strength
    KHAL_PORT_ATTR_DSE_HIGH = 1
} khal_port_attr_en_t;

/// PORT Control Register interrupt configuration constants.
typedef enum khal_port_int_config_t {
    KHAL_PORT_INT_DMA_DIS   = 0,        ///< interrupt and DMA requests disabled
    KHAL_PORT_DMA_RE_EN     = 1,        ///< DMA request on rising edge
    KHAL_PORT_DMA_FE_EN     = 2,        ///< DMA request on falling edge
    KHAL_PORT_DMA_BOTH_EN   = 3,        ///< DMA request on both edges
    KHAL_PORT_INT_0_EN      = 8,        ///< interrupt when logic 0
    KHAL_PORT_INT_RE_EN     = 9,        ///< interrupt on rising edge
    KHAL_PORT_INT_FE_EN     = 10,       ///< interrupt on falling edge
    KHAL_PORT_INT_BOTH_EN   = 11,       ///< interrupt on both edges
    KHAL_PORT_INT_1_EN      = 12,       ///< interrupt when logic 1
    KHAL_PORT_INT_DMA_ERR   = 13
} khal_port_int_config_t;


// ==== Defines ====

/// Set a GPIO pin to a value (0, 1). Port is one of A | B | C | D | E.
#define KHAL_PORT_PIN_PUT(port, pin, val)   FGPIO##port##_PDOR = ((FGPIO##port##_PDOR & ~(1 << pin)) | (val << pin))

/// Set a GPIO pin to a 1. Port is one of A | B | C | D | E.
#define KHAL_PORT_PIN_SET(port, pin)        FGPIO##port##_PSOR = (1 << pin)

/// Set a GPIO pin to a 0. Port is one of A | B | C | D | E.
#define KHAL_PORT_PIN_CLR(port, pin)        FGPIO##port##_PCOR = (1 << pin)

/// Toggle a GPIO pin. Port is one of A | B | C | D | E.
#define KHAL_PORT_PIN_TOG(port, pin)        FGPIO##port##_PTOR = (1 << pin)

/// Return the state of a GPIO pin. Port is one of A | B | C | D | E.
#define KHAL_PORT_PIN_GET(port, pin)        ((FGPIO##port##_PDIR & (1 << pin)) >> pin)

/// Set the direction of a GPIO pin. Port is one of A | B | C | D | E.
#define KHAL_PORT_PIN_DIR(port, pin, dir)   FGPIO##port##_PDDR = ((FGPIO##port##_PDDR & ~(1 << pin)) | (dir << pin))


// ==== API ====

/// Initialize a port pin.
void khal_port_Init(PORT_MemMapPtr port, uint32_t pin, uint32_t               mux,
                                                       khal_port_data_dir_t   dir,
                                                       uint32_t               output_level,
                                                       khal_port_attr_en_t    pe,
                                                       khal_port_attr_en_t    ps,
                                                       khal_port_attr_en_t    sre,
                                                       khal_port_attr_en_t    pfe,
                                                       khal_port_attr_en_t    ode,
                                                       khal_port_attr_en_t    dse,
                                                       khal_port_int_config_t irqc);

/// Disable a port pin by setting the mux select to 0
void khal_port_Disable(PORT_MemMapPtr port, uint32_t pin);


#endif  /* _khal_port_H_ */


