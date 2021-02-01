
/**
 *  \file lockless.h
 *  \brief A collection of thread-safe lockless data structures.
 *
 *  LL_BIT_VECTOR: An arbitrary length set of bits.
 *  LL_QUEUE:      A fixed size ring buffer with 32 bit elements.
 *
 *  The thread-safe lockless data structures rely on a compare-and-swap primitive (CAS).
 *  Arm CM3/CM4 processors implement CAS using ldrex/strex and do not disable interrupts.
 *  Arm CM0/CM0+ processors do not have an atomic operator and must briefly disable interrupts.
 *
 *  (c) Copyright 2016 DDPA LLC
 *  ALL RIGHTS RESERVED.
 *
 */


#ifndef _lockless_H_
#define _lockless_H_

#include  <stdint.h>




/**
 * @param addr  The address of the variable to be changed.
 * @param expected  The expected current value in @p *addr that will be changed.
 * @param store  The value to store in @p *addr iff @p *addr == @p expected.
 * @return 0 if swap succeeded, 1 otherwise.
 */
#if (__CORTEX_M == 0)
static inline int llAtomicCAS(volatile void ** addr, void * expected, void * store) {
    int ret = 1;

    LLESS_LOCK;
    if (*addr == expected) {
      *addr = store;
       ret  = 0;
    }
    LLESS_END_LOCK;
    return ret;
}
#define LLESS_LOCK     uint32_t primask_save = __get_PRIMASK(); DisableInterrupts
#define LLESS_END_LOCK __set_PRIMASK(primask_save)
#elif ((__CORTEX_M == 3) || (__CORTEX_M == 4))
static inline int llAtomicCAS(volatile void ** addr, void * expected, void * store) {

    if (__ldrex(addr) != expected) {
        return 1;
    }

    return (__strex(store, addr));
}
#else
    #warning "Only ARM M0/M0+/M3/M4 are supported"
#endif



/*****************************************************************************

    LL_BIT_VECTOR: An arbitrary length set of bits.

        For bit vectors longer than 32 bits, atomic operation is realized
        only on the word-aligned 32 bits containing the targeted bit.
        The bit vector name is globally visible.
        NEW_LL_BIT_VECTOR must be invoked at file scope.

        Usage Example:
        NEW_LL_BIT_VECTOR(bv_name, bv_size);
        uint32_t = llbvstate(bv_name, uint32_t whichbit);
        llbvSet(bv_name, uint32_t whichbit);


   Revision History:
       03/11/16  Initial release

 *****************************************************************************/

/// Bit Vector.
typedef struct ll_bit_vector_t {
    uint32_t      size;       ///< Bits in vector.
    uint32_t   array[];       ///< Pointer to bit vector.
} const ll_bit_vector_t;

/// Macro to define the storage for a bit vector. bv_name has global scope.
#define NEW_LL_BIT_VECTOR(bv_name, bv_size)                                   \
volatile uint32_t bv_name##_array[ ((bv_size-1)/32)+1 ] = { 0 };              \
ll_bit_vector_t bv_name = { bv_size, &bv_name##_array }


// ==== Bit Vector Functions ====

/// Set a bit (lsb = 0). Operation is atomic only for the word aligned 32 bits containing set_bit.
/// It is a checked run time error for set_bit to be >= a.size.
void  llbvSet(ll_bit_vector_t *a, uint32_t set_bit);

/// Clear a bit (lsb = 0). Operation is atomic only for the word aligned 32 bits containing set_bit.
/// It is a checked run time error for set_bit to be >= a.size.
void  llbvClr(ll_bit_vector_t *a, uint32_t clr_bit);

/// Find the state of a bit (lsb = 0).
/// \return the state of the whichBit (1 or 0).
uint32_t  llbvState(ll_bit_vector_t *a, uint32_t whichBit);

/// Find the position of the first one in the bitmap (lsb = 1).
/// \return the position of the first one bit (msb = a.size, lsb = 1), or 0 if there are no ones.
uint32_t  llbvFF1(ll_bit_vector_t *a);

/// Logical AND of two bitmaps. Bit vectors a and b are not locked and may change during this operation.
/// \return the logical AND of bit vectors a and b.
void  llbvAND(ll_bit_vector_t *rslt, ll_bit_vector_t *a, ll_bit_vector_t *b);

/// Logical OR of two bitmaps. Bit vectors a and b are not locked and may change during this operation.
/// \return the logical OR of bit vectors a and b.
void  llbvOR(ll_bit_vector_t *rslt, ll_bit_vector_t *a, ll_bit_vector_t *b);

/// Copy a bitmap. Bit vector a is not locked and may change during this operation.
/// \return a copy of bit vector a.
void  llbvCopy(ll_bit_vector_t *copy, ll_bit_vector_t *a);



/*****************************************************************************

    LL_QUEUE: A fixed size ring buffer with 32 bit elements.

        The queue is allocated at compile-time and cannot be resized or dynamically created.
        The queue name is globally visible.
        Queue elements are void * or cast to void *.
        NEW_LL_QUEUE must be invoked at file scope.

        NOTE: The queue requires a static initializer (i.e. copy in Flash) the
              same size as the queue itself.

        Usage Example:
        NEW_LLESS_QUEUE(q_name, size);
        success = llqPut(q_name, (void *) myval);
        success = llqGet(q_name, (void **) &myval);

   Revision History:
       03/10/16  Initial release

 *****************************************************************************/


typedef struct ll_queue_t {
    uint32_t           head;       ///< Next available for put
    uint32_t           tail;       ///< Next valid for get
    uint32_t           size;       ///< Must be power of 2
    void *          array[];       ///< Pointer to queue storage array
    ll_bit_vector_t * valid;       ///< Valid bits for each queue entry (1 = valid)
} volatile ll_queue_t;

#define NEW_LL_QUEUE(q_name, q_size)                                          \
volatile void * q_name##_array[q_size] = { 0 };                               \
NEW_LL_BIT_VECTOR(q_name##_bv, q_size)                                        \
ll_queue_t q_name = { 0, 0, size, q_name##_array, &q_name##_bv }              \


// ==== Queue Functions ====

/// Put val into queue. The put operation is thread-safe..
/// \return TRUE if the queue was not full and val was successfully added.
bool  llqPut(ll_queue_t *queue, void * val);

/// Get the next value from the queue. The get operation is thread-safe..
/// \return TRUE if the queue was not empty and val was successfully fetched.
bool  llqGet(ll_queue_t *queue, void ** val);



#endif  /* _lockless_H_ */
