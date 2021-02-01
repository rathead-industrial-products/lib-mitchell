/******************************************************************************

    cpu.c - Processor specific functions.

    COPYRIGHT NOTICE: (c) 2016 DDPA LLC
    All Rights Reserved

******************************************************************************/

#include "cpu.h"


#if defined (UNIT_TEST)
int cpuCAS(uint32_t volatile * const addr, uint32_t const expected, uint32_t const store) {
    int rslt;
    test_preCAS();
    rslt = test_CAS_OP(addr, expected, store);
    test_postCAS();
    return (rslt);
}
#endif

#if (defined (UNIT_TEST)) || (__CORTEX_M == 0)
int cpuCLZ(uint32_t const x) {
    static uint8_t const clz_lkup[] = {
        32U, 31U, 30U, 30U, 29U, 29U, 29U, 29U,
        28U, 28U, 28U, 28U, 28U, 28U, 28U, 28U
    };
    uint32_t n;

    if (x >= (1U << 16)) {
        if (x >= (1U << 24)) {
            if (x >= (1 << 28)) {
                n = 28U;
            }
            else {
                n = 24U;
            }
        }
        else {
            if (x >= (1U << 20)) {
                n = 20U;
            }
            else {
                n = 16U;
            }
        }
    }
    else {
        if (x >= (1U << 8)) {
            if (x >= (1U << 12)) {
                n = 12U;
            }
            else {
                n = 8U;
            }
        }
        else {
            if (x >= (1U << 4)) {
                n = 4U;
            }
            else {
                n = 0U;
            }
        }
    }
    return ((int) clz_lkup[x >> n] - n);
}
#endif


#if !(defined (UNIT_TEST))
#if (__CORTEX_M == 0)
int cpuCAS(uint32_t volatile * const addr, uint32_t const expected, uint32_t const store) {
    int rslt = 1;

    CPU_LOCK;
    if (*addr == expected) {
      *addr = store;
       rslt = 0;
    }
    CPU_END_LOCK;
    return (rslt);
}

#elif ((__CORTEX_M == 3) || (__CORTEX_M == 4))
int cpuCAS(uint32_t volatile * const addr, uint32_t const expected, uint32_t const store) {

    if (__LDREXW(addr) != expected) {
        return 1;
    }
    return (__STREXW(store, addr));
}

int cpuCLZ(uint32_t const x) {
  return ((uint32_t) __CLZ(x));
}

#else
    #warning "Only ARM M0/M0+/M3/M4 are supported"
#endif  /*  __CORTEX_M selection */
#endif  /* !(defined (UNIT_TEST)) */
