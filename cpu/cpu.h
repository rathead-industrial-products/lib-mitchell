/**
 *
 *  @file  cpu.h
 *  @brief Functions requiring specific processor-level architectural support.
 *
 *  The included file derivative.h must define (or must include a definition
 *  of the processor architecture __CORTEX_M, which must be one of 0, 3, or 4.
 *
 *  COPYRIGHT NOTICE: (c) 2016 DDPA LLC
 *  All Rights Reserved
 *
 */

#ifndef _cpu_H_
#define _cpu_H_

#include  <stdint.h>
#include "derivative.h"

#if defined (__CORTEX_M)
    #include "core_cmFunc.h"    // cmsis intrinsics

    #define CPU_LOCK     uint32_t primask_save = __get_PRIMASK(); __disable_irq()
    #define CPU_END_LOCK __set_PRIMASK(primask_save)

    #if (__CORTEX_M == 0)
        #include "core_cm0plus.h"
    #elif ((__CORTEX_M == 3) || (__CORTEX_M == 4))
        #include "core_cm4.h"
    #else
        #warning "Only ARM M0/M0+/M3/M4 are supported"
    #endif
#endif

/*
 * Unit test are by definition single threaded and may be run on
 * a host (i.e. non ARM) computer, so no locking.
 *
 * The CAS operation has hooks installed before, during, and after
 * to allow simulation of context changes occurring at any time.
 */
#if defined (UNIT_TEST)
    #define CPU_LOCK
    #define CPU_END_LOCK

void test_preCAS(void);
int  test_CAS_OP(uint32_t volatile * const addr, uint32_t const expected, uint32_t const store);
void test_postCAS(void);


#endif




/**
 *  Atomic Compare-And-Swap atomic operation for lockless structures.
 *
 *  The M0/M0+ do not have an atomic operator, so interrupts are disabled for
 *  four instructions every time this function is called.
 *
 *  The M3/M4 have a LL/SC construct that is used to implement CAS.
 *
 *  Both implementations are free of the ABA hazard. Inputs are cast to a
 *  uint32_t type to allow casting to any 32 bit value.
 *
 *  \param [in]   addr      Pointer to the variable to be updated.
 *  \param [in]   expected  Value contained in *addr unless overwritten by another thread.
 *  \param [in]   store     Value to assign to *addr if *addr == expected.
 *  \return       0 if *addr successfully updated, or 1 if it was changed by another thread.
 */
int cpuCAS(uint32_t volatile * const addr, uint32_t const expected, uint32_t const store);

/// Count Leading Zeros in a 32 bit value.
/// \return   Bit position of first one bit (msb=0, lsb=31) or 32 if no bits set.
int cpuCLZ(uint32_t const x);




#endif  /* _cpu_H_ */
