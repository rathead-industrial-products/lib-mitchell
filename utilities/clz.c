/*
 *    (c) Copyright 2016 DDPA LLC
 *    ALL RIGHTS RESERVED.
 *
 *    Function to count leading zeros.
 *
 *    Targeted at ARM Cortex M0, M0+ processors.
 *
 */

#include  <stdint.h>

#pragma GCC optimize ("O3")


#if (__CORTEX_M != 0) /* Confirm processor type */
#error "Startup code targeted at M0 or M0+ processor."
#endif


/*
 *
 *
 */

#if (0)
uint32_t clz(uint32_t x) {
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
    return (uint32_t)clz_lkup[x >> n] - n;
}



#else


uint32_t clz(uint32_t x) {
    static const uint8_t LUT_CLZ[16] = { 4, 3, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 };

    __asm volatile (
            "ldr  r1, =0xffff0000   \n"   // mask to test upper 16 bits
            "mov  r2, #0            \n"   // running count of leading zeros
            "tst  r0, r1            \n"
            "bne  _L0               \n"   // branch if upper halfword has one or more set bits
            "lsl  r0, r0, #16       \n"   // upper halfward is all zeros
            "add  r2, #16           \n"   // add to count of leading zeros
        "_L0:                       \n"
            "lsl  r1, #8            \n"   // mask to test upper 8 bits
            "tst  r0, r1            \n"
            "bne  _L1               \n"   // branch if upper byte has one or more set bits
            "lsl  r0, r0, #8        \n"   // upper byte is all zeros
            "add  r2, #8            \n"   // add to count of leading zeros
        "_L1:                       \n"
            "lsl  r1, #8            \n"   // mask to test upper 4 bits
            "tst  r0, r1            \n"
            "bne  _L2               \n"   // branch if upper nibble has one or more set bits
            "lsl  r0, r0, #4        \n"   // upper nibble is all zeros
            "add  r2, #4            \n"   // add to count of leading zeros
        "_L2:                       \n"
            //"ldr r1, =LUT_CLZ       \n"   // LUT: 4 bits in, number of leading zeros out
            //"ldr r1, [r1, r0]       \n"   // table look-up
            "add r0, r1             \n"   // add table value to leading zero count
    );


}
#endif
