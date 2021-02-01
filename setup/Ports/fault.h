/*****************************************************************************
 *
    (c) Copyright 2016 DDPA LLC
    ALL RIGHTS RESERVED.

    fault.h - Defines a data structure to hold all registers that can be
              recovered after a fault for post-mortem analysis.

 *****************************************************************************/

#ifndef _fault_H_
#define _fault_H_

#define CORE_TYPE_M0      0
#define CORE_TYPE_M4      4

#include  <stdint.h>

#if (__CORTEX_M == 0)
struct fault_t {
    struct {
        uint32_t  r0;
        uint32_t  r1;
        uint32_t  r2;
        uint32_t  r3;
        uint32_t  r12;
        uint32_t  lr;
        uint32_t  sp;
        uint32_t  pc;
    } stacked_registers;
    union {
        struct {
            uint32_t  cur_exception_num : 6;
            uint32_t  rsvd6_23          : 18;
            uint32_t  thumb_state       : 1;
            uint32_t  rsvd25_27         : 3;
            uint32_t  f_overflow        : 1;
            uint32_t  f_carry           : 1;
            uint32_t  f_zero            : 1;
            uint32_t  f_negative        : 1;
        } fields;
        uint32_t  reg;
    } psr;
    union {
        struct {
            uint32_t  rsvd0_11          : 12;
            uint32_t  vectpending       : 6;
            uint32_t  rsvd18_24         : 7;
            uint32_t  pendstclr         : 1;
            uint32_t  pendstset         : 1;
            uint32_t  pendsvclr         : 1;
            uint32_t  pendsvset         : 1;
            uint32_t  rsvd29_30         : 2;
            uint32_t  nmipendset        : 1;
        } fields;
        uint32_t  reg;
    } icsr;
    union {
        struct {
            uint32_t  memfaultact       : 1;
            uint32_t  busfaultact       : 1;
            uint32_t  rsvd3             : 1;
            uint32_t  usgfaultact       : 1;
            uint32_t  rsvd4             : 3;
            uint32_t  svcallact         : 1;
            uint32_t  monitoract        : 1;
            uint32_t  rsvd9             : 1;
            uint32_t  pendsvact         : 1;
            uint32_t  systickact        : 1;
            uint32_t  usgfaultpended    : 1;
            uint32_t  memfaultpended    : 1;
            uint32_t  busfaultpended    : 1;
            uint32_t  svcallpended      : 1;
            uint32_t  memfaultena       : 1;
            uint32_t  busfaultena       : 1;
            uint32_t  usgfaultena       : 1;
            uint32_t  rsvd19            : 13;
        } fields;
        uint32_t  reg;
    } shcsr;
};
#endif  /* __CORTEX_M == 0 */

#if (__CORTEX_M == 4)
struct fault_t {
    struct {
        uint32_t  r0;
        uint32_t  r1;
        uint32_t  r2;
        uint32_t  r3;
        uint32_t  r12;
        uint32_t  lr;
        uint32_t  sp;
        uint32_t  pc;
    } stacked_registers;
    union {
        struct {
            uint32_t  cur_exception_num : 6;
            uint32_t  rsvd6_23          : 18;
            uint32_t  thumb_state       : 1;
            uint32_t  rsvd25_27         : 3;
            uint32_t  f_overflow        : 1;
            uint32_t  f_carry           : 1;
            uint32_t  f_zero            : 1;
            uint32_t  f_negative        : 1;
        } fields;
        uint32_t  reg;
    } psr;
    union {
        struct {
            uint32_t  vectactive        : 9;
            uint32_t  rsvd9             : 2;
            uint32_t  rettobase         : 1;
            uint32_t  vectpending       : 10;
            uint32_t  isrpending        : 1;
            uint32_t  rsvd23            : 1;
            uint32_t  rsvd24            : 1;
            uint32_t  pendstclr         : 1;
            uint32_t  pendstset         : 1;
            uint32_t  pendsvclr         : 1;
            uint32_t  pendsvset         : 1;
            uint32_t  rsvd29            : 2;
            uint32_t  nmipendset        : 1;
        } fields;
        uint32_t  reg;
    } icsr;
    union {
        struct {
            uint32_t  memfaultact       : 1;
            uint32_t  busfaultact       : 1;
            uint32_t  rsvd3             : 1;
            uint32_t  usgfaultact       : 1;
            uint32_t  rsvd4             : 3;
            uint32_t  svcallact         : 1;
            uint32_t  monitoract        : 1;
            uint32_t  rsvd9             : 1;
            uint32_t  pendsvact         : 1;
            uint32_t  systickact        : 1;
            uint32_t  usgfaultpended    : 1;
            uint32_t  memfaultpended    : 1;
            uint32_t  busfaultpended    : 1;
            uint32_t  svcallpended      : 1;
            uint32_t  memfaultena       : 1;
            uint32_t  busfaultena       : 1;
            uint32_t  usgfaultena       : 1;
            uint32_t  rsvd19            : 13;
        } fields;
        uint32_t  reg;
    } shcsr;
    union {
        struct {
            union {
                struct {
                    uint8_t iaccviol    : 1;
                    uint8_t daccviol    : 1;
                    uint8_t rsvd2       : 1;
                    uint8_t munstkerr   : 1;
                    uint8_t mstkerr     : 1;
                    uint8_t mlsperr     : 1;
                    uint8_t rsvd6       : 1;
                    uint8_t mmarvalid   : 1;
                } fields;
                uint8_t reg;
            } mmfsr;
            union {
                struct {
                    uint8_t ibuserr     : 1;
                    uint8_t preciserr   : 1;
                    uint8_t impreciserr : 1;
                    uint8_t unstkrerr   : 1;
                    uint8_t stkerr      : 1;
                    uint8_t lpserr      : 1;
                    uint8_t rsvd6       : 1;
                    uint8_t bfarvalid   : 1;
                } fields;
                uint8_t reg;
            } bfsr;
            union {
                struct {
                    uint8_t undefinstr  : 1;
                    uint8_t invstate    : 1;
                    uint8_t invpc       : 1;
                    uint8_t nocp        : 1;
                    uint8_t rsvd4       : 4;
                    uint8_t unaligned   : 1;
                    uint8_t divbyzero   : 1;
                    uint8_t rsvd10      : 6;
                } fields;
                uint16_t reg;
            } ufsr;
        } subreg;
        uint32_t  reg;
    } cfsr;
    union {
        struct {
            uint32_t  rsvd0             : 1;
            uint32_t  vecttbl           : 1;
            uint32_t  rsvd2             : 28;
            uint32_t  forced            : 1;
            uint32_t  debugevt          : 1;
        } fields;
        uint32_t  reg;
    } hfsr;
    union {
        uint32_t  reg;
    } bfar;    union {
        uint32_t  reg;
    } afsr;
};
#endif  /* __CORTEX_M == 4 */


#endif  /* _fault_H_ */
