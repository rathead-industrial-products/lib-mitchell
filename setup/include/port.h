/*******************************************************************************

    port.h - MPU PORT and GPIO ata structures and macros.

    COPYRIGHT NOTICE: (c) 2014 DDPA LLC
    All Rights Reserved

 ******************************************************************************/

#ifndef _port_H_
#define _port_H_

#include "derivative.h"

typedef struct {
    uint32_t  D0     : 1;
    uint32_t  D1     : 1;
    uint32_t  D2     : 1;
    uint32_t  D3     : 1;
    uint32_t  D4     : 1;
    uint32_t  D5     : 1;
    uint32_t  D6     : 1;
    uint32_t  D7     : 1;
    uint32_t  D8     : 1;
    uint32_t  D9     : 1;
    uint32_t  D10    : 1;
    uint32_t  D11    : 1;
    uint32_t  D12    : 1;
    uint32_t  D13    : 1;
    uint32_t  D14    : 1;
    uint32_t  D15    : 1;
    uint32_t  D16    : 1;
    uint32_t  D17    : 1;
    uint32_t  D18    : 1;
    uint32_t  D19    : 1;
    uint32_t  D20    : 1;
    uint32_t  D21    : 1;
    uint32_t  D22    : 1;
    uint32_t  D23    : 1;
    uint32_t  D24    : 1;
    uint32_t  D25    : 1;
    uint32_t  D26    : 1;
    uint32_t  D27    : 1;
    uint32_t  D28    : 1;
    uint32_t  D29    : 1;
    uint32_t  D30    : 1;
    uint32_t  D31    : 1;
} *GPIO_BitMap;

#define PTADOR    ((GPIO_BitMap) (&GPIOA_PDOR))
#define PTASOR    ((GPIO_BitMap) (&GPIOA_PSOR))
#define PTACOR    ((GPIO_BitMap) (&GPIOA_PCOR))
#define PTATOR    ((GPIO_BitMap) (&GPIOA_PTOR))
#define PTADIR    ((GPIO_BitMap) (&GPIOA_PDIR))
#define PTADDR    ((GPIO_BitMap) (&GPIOA_PDDR))

#define PTBDOR    ((GPIO_BitMap) (&GPIOB_PDOR))
#define PTBSOR    ((GPIO_BitMap) (&GPIOB_PSOR))
#define PTBCOR    ((GPIO_BitMap) (&GPIOB_PCOR))
#define PTBTOR    ((GPIO_BitMap) (&GPIOB_PTOR))
#define PTBDIR    ((GPIO_BitMap) (&GPIOB_PDIR))
#define PTBDDR    ((GPIO_BitMap) (&GPIOB_PDDR))

#define PTCDOR    ((GPIO_BitMap) (&GPIOC_PDOR))
#define PTCSOR    ((GPIO_BitMap) (&GPIOC_PSOR))
#define PTCCOR    ((GPIO_BitMap) (&GPIOC_PCOR))
#define PTCTOR    ((GPIO_BitMap) (&GPIOC_PTOR))
#define PTCDIR    ((GPIO_BitMap) (&GPIOC_PDIR))
#define PTCDDR    ((GPIO_BitMap) (&GPIOC_PDDR))

#define PTDDOR    ((GPIO_BitMap) (&GPIOD_PDOR))
#define PTDSOR    ((GPIO_BitMap) (&GPIOD_PSOR))
#define PTDCOR    ((GPIO_BitMap) (&GPIOD_PCOR))
#define PTDTOR    ((GPIO_BitMap) (&GPIOD_PTOR))
#define PTDDIR    ((GPIO_BitMap) (&GPIOD_PDIR))
#define PTDDDR    ((GPIO_BitMap) (&GPIOD_PDDR))

#define PTEDOR    ((GPIO_BitMap) (&GPIOE_PDOR))
#define PTESOR    ((GPIO_BitMap) (&GPIOE_PSOR))
#define PTECOR    ((GPIO_BitMap) (&GPIOE_PCOR))
#define PTETOR    ((GPIO_BitMap) (&GPIOE_PTOR))
#define PTEDIR    ((GPIO_BitMap) (&GPIOE_PDIR))
#define PTEDDR    ((GPIO_BitMap) (&GPIOE_PDDR))




#endif  /* _port_H_ */
