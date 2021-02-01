/*
 *    (c) Copyright 2016 DDPA LLC
 *    ALL RIGHTS RESERVED.
 *
 *    Implements a compare-and-swap atomic operation. Does NOT test for
 *    an ABA error condition.
 *
 *    Targeted at ARM Cortex M0, M0+, M3, and M4 processors.
 *
 */

#include  <stdint.h>
#include  "contract.h"

#if (__CORTEX_M == 0)
/* return 0 if CAS operation succeeded */
static inline int atomic_CAS(volatile int32_t * addr, int32_t expected, int32_t store) {
  int ret = 1;

   LOCK;
   if (*addr == expected) {
      *addr = store;
      ret = 0;
   }
   END_LOCK;
   return ret;
}
#endif


#if ((__CORTEX_M == 3) || (__CORTEX_M == 4))
/* return 0 if CAS operation succeeded  */
static inline int atomic_CAS(volatile int32_t * addr, int32_t expected, int32_t store) {

    if (__ldrex(addr) != expected) {
      return 1;
    }

    return (__strex(store, addr));
}
#endif



/*
 * Return the incremented value of a variable, updated atomically
 */
static inline int atomic_INC(volatile int32_t * p_var) {
    int32_t local_var;

    do {
        local_var = *p_var;
    } while (atomic_CAS(p_var, local_var, local_var + 1));

   return (local_var + 1);
}


/*
 * Return the incremented value of a variable saturated at a
 * maximum value, updated atomically
 */
static inline int atomic_INC_SAT(volatile int32_t * p_var, int32_t max) {
    int32_t local_var;

    do {
        local_var = *p_var;
        if (local_var == max) {
            --local_var;
            break;
        }
    } while (atomic_CAS(p_var, local_var, local_var + 1));

   return (local_var + 1);
}


/*
 * Return the decremented value of a variable, updated atomically
 */
static inline int atomic_DEC(volatile int32_t * p_var) {
    int32_t local_var;

    do {
        local_var = *p_var;
    } while (atomic_CAS(p_var, local_var, local_var - 1));

   return (local_var - 1);
}


/*
 * Return the decremented value of a variable saturated at a
 * minimum value, updated atomically
 */
static inline int atomic_DEC_SAT(volatile int32_t * p_var, int32_t min) {
    int32_t local_var;

    do {
        local_var = *p_var;
        if (local_var == min) {
            ++local_var;
            break;
        }
    } while (atomic_CAS(p_var, local_var, local_var - 1));

   return (local_var - 1);
}



