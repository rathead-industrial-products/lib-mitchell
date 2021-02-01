/**
 *
 *  samd_port.h
 *  Hardware Abstraction Layer for Microchip SAMD MCU PORT module.
 *
 *  COPYRIGHT NOTICE: (c) 2019 DDPA LLC
 *  All Rights Reserved
 *
 */


#ifndef _samd_port_H_
#define _samd_port_H_

#include  <stdbool.h>
#include  <stdint.h>
#include  "sam.h"


#define SAMD_PORTA    (0)   // PORT->Group[0]
#define SAMD_PORTB    (1)   // PORT->Group[1]


// ==== Typedefs ====

// External pin interrupt callback
typedef void (*samd_port_int_callback_fn_t) (void *argument);

// Port group number and pin concatenated
typedef uint32_t samd_port_pin_t ;

// Input enable.
typedef enum { SAMD_PORT_INPUT_DISABLE = 0,
               SAMD_PORT_INPUT_ENABLE  = 1
} samd_port_ie_t;

// Output enable.
typedef enum { SAMD_PORT_OUTPUT_DISABLE = 0,
               SAMD_PORT_OUTPUT_ENABLE  = 1
} samd_port_oe_t;

// Drive strength.
typedef enum { SAMD_PORT_DRIVE_STRENGTH_NORMAL = 0,
               SAMD_PORT_DRIVE_STRENGTH_HIGH   = 1
} samd_port_ds_t;

// Pull mode.
typedef enum { SAMD_PORT_PULL_OFF  = 0,
               SAMD_PORT_PULL_ON   = 1
} samd_port_pull_mode_t;

// Peripheral mux selector.
typedef enum {
    SAMD_PORT_PERIPHERAL_FUNC_GPIO = -1,
    SAMD_PORT_PERIPHERAL_FUNC_A    =  0,
    SAMD_PORT_PERIPHERAL_FUNC_B    =  1,
    SAMD_PORT_PERIPHERAL_FUNC_C    =  2,
    SAMD_PORT_PERIPHERAL_FUNC_D    =  3,
    SAMD_PORT_PERIPHERAL_FUNC_E    =  4,
    SAMD_PORT_PERIPHERAL_FUNC_F    =  5,
    SAMD_PORT_PERIPHERAL_FUNC_G    =  6,
    SAMD_PORT_PERIPHERAL_FUNC_H    =  7,
    SAMD_PORT_PERIPHERAL_FUNC_I    =  8,
    SAMD_PORT_PERIPHERAL_FUNC_J    =  9,
    SAMD_PORT_PERIPHERAL_FUNC_K    =  10,
    SAMD_PORT_PERIPHERAL_FUNC_L    =  11,
    SAMD_PORT_PERIPHERAL_FUNC_M    =  12,
    SAMD_PORT_PERIPHERAL_FUNC_N    =  13
} samd_port_peripheral_mux_t;

// Interrupt behavior
typedef enum { SAMD_PORT_INT_NONE=0, SAMD_PORT_INT_RISE, SAMD_PORT_INT_FALL, SAMD_PORT_INT_BOTH, SAMD_PORT_INT_HIGH, SAMD_PORT_INT_LOW } samd_port_int_t;




// ==== Defines ====

// Macros to combine and extract the port group and pin references
#define SAMD_PORT_PIN(port, pin)                        (((port) << 16) | ((pin) & 0x1f))
#define SAMD_PORT(port_pin)                             PORT->Group[(port_pin) >> 16]
#define SAMD_PIN(port_pin)                              ((port_pin) & 0x1f)

// Return the state of a GPIO pin.
#define SAMD_PORT_PIN_IN(port_pin)                      ((SAMD_PORT(port_pin).IN.reg & (1 << SAMD_PIN(port_pin))) >> SAMD_PIN(port_pin))

// Set a GPIO pin to a value (0, 1).
#define SAMD_PORT_PIN_PUT(port_pin, val)                (SAMD_PORT(port_pin).OUT.reg = (SAMD_PORT(port_pin).OUT.reg & ~(1 << SAMD_PIN(port_pin))) | (val << SAMD_PIN(port_pin)))

// Set a GPIO pin to a 1.
#define SAMD_PORT_PIN_SET(port_pin)                     (SAMD_PORT(port_pin).OUTSET.reg = (1 << SAMD_PIN(port_pin)))

// Set a GPIO pin to a 0.
#define SAMD_PORT_PIN_CLR(port_pin)                     (SAMD_PORT(port_pin).OUTCLR.reg = (1 << SAMD_PIN(port_pin)))

// Toggle a GPIO pin.
#define SAMD_PORT_PIN_TOG(port_pin)                     (SAMD_PORT(port_pin).OUTTGL.reg = (1 << SAMD_PIN(port_pin)))

// Enable a GPIO pin input buffer.
#define SAMD_PORT_PIN_INPUT_ENABLE(port_pin)            (SAMD_PORT(port_pin).PINCFG[SAMD_PIN(port_pin)].reg |= PORT_PINCFG_INEN)

// Disable a GPIO pin input buffer.
#define SAMD_PORT_PIN_INPUT_DISABLE(port_pin)           (SAMD_PORT(port_pin).PINCFG[SAMD_PIN(port_pin)].reg &= ~PORT_PINCFG_INEN)

// Enable a GPIO pin output driver.
#define SAMD_PORT_PIN_OUTPUT_ENABLE(port_pin)           (SAMD_PORT(port_pin).DIRSET.reg = (1 << SAMD_PIN(port_pin)))

// Disable a GPIO pin output driver.
#define SAMD_PORT_PIN_OUTPUT_DISABLE(port_pin)          (SAMD_PORT(port_pin).DIRCLR.reg = (1 << SAMD_PIN(port_pin)))

// Set the drive strength of an output buffer to high.
#define SAMD_PORT_PIN_DRIVE_STRENGTH_HIGH(port_pin)     (SAMD_PORT(port_pin).PINCFG[SAMD_PIN(port_pin)].reg |= PORT_PINCFG_DRVSTR)

// Set the drive strength of an output buffer to normal.
#define SAMD_PORT_PIN_DRIVE_STRENGTH_NORMAL(port_pin)   (SAMD_PORT(port_pin).PINCFG[SAMD_PIN(port_pin)].reg &= ~PORT_PINCFG_DRVSTR)

// Enable a GPIO pin pull buffer.
#define SAMD_PORT_PIN_PULL_ENABLE(port_pin)             (SAMD_PORT(port_pin).PINCFG[SAMD_PIN(port_pin)].reg |= PORT_PINCFG_PULLEN)

// Disable a GPIO pin pull buffer.
#define SAMD_PORT_PIN_PULL_DISABLE(port_pin)            (SAMD_PORT(port_pin).PINCFG[SAMD_PIN(port_pin)].reg &= ~PORT_PINCFG_PULLEN)



// ==== API ====

// Initialize a port pin.
void samd_port_Init(samd_port_pin_t port_pin,
        uint32_t                    output_level,
        samd_port_oe_t              oe,
        samd_port_ie_t              ie,
        samd_port_ds_t              ds,
        samd_port_pull_mode_t       pm,
        samd_port_peripheral_mux_t  pin_func);

// Interrupt behavior
void samd_port_IntConfig(samd_port_pin_t  port_pin,
             uint32_t                     extint,
             samd_port_int_t              sense,
             bool                         filter,
             samd_port_int_callback_fn_t  cb);

// Disable a port pin to its inactive lowest power state
void samd_port_Disable(samd_port_pin_t port_pin);

// Set a port to an alternate function.
void samd_port_AltFunc(samd_port_pin_t port_pin, samd_port_peripheral_mux_t func);


#endif  /* _samd_port_H_ */


