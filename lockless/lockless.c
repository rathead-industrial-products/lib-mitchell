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


#include <stdint.h>
#include "lockless.h"



//  ==== Bit Vector Functions ====

#define EEX_LL_BV_WORD(bit)      ((bit-1) / 32)                               ///< the word containing bit in a bitmap of size > 32
#define EEX_LL_BV_BIT(bit)       ((bit-1) - (32 * EEX_LL_BV_WORD(bit)))       ///< the bit position within a word

uint32_t  _llwordCLZ(uint32_t x) {
    uint32_t  n;
    #if ((__CORTEX_M == 3) || (__CORTEX_M == 4))
    {
        return __asm ("clz %0, %1" : "=r"(n) : "r"(x))
    }
    #else
    /* algorithm from http://embeddedgurus.com/state-space/tag/arm-cortex-m0 */
    {
        static uint8_t const clz_lkup[] = {
             32, 31, 30, 30, 29, 29, 29, 29,
             28, 28, 28, 28, 28, 28, 28, 28
         };

         if (x >= (1 << 16)) {
             if (x >= (1 << 24)) {
                 if (x >= (1 << 28)) { n = 28; }
                 else {                n = 24; }
             }
             else {
                 if (x >= (1 << 20)) { n = 20; }
                 else {                n = 16; }
             }
         }
         else {
             if (x >= (1 << 8)) {
                 if (x >= (1 << 12)) { n = 12; }
                 else {                n = 8; }
             }
             else {
                 if (x >= (1 << 4))  { n = 4; }
                 else {                n = 0; }
             }
         }
         return (uint32_t) clz_lkup[x >> n] - n;
    }
    #endif
}

void  llbvSet(ll_bit_vector_t *a, uint32_t set_bit) {
    uint32_t  bv_word_old, bv_word_new;
    REQUIRE (set_bit <= a->size);
    do {
        bv_word_old = a->array[EEX_LL_BV_WORD(setBit)];
        bv_word_new = bv_word_old | (1 << EEX_LL_BV_BIT(setBit));
    } while (llAtomicCAS(&(a->array[EEX_LL_BV_WORD(setBit)]), bv_word_old, bv_word_new));
}

void  llbvClr(ll_bit_vector_t *a, uint32_t clr_bit) {
    REQUIRE (set_bit <= a->size);
    do {
        bv_word_old = a->array[EEX_LL_BV_WORD(clr_bit)];
        bv_word_new = bv_word_old &= ~(1 << EEX_LL_BV_BIT(clrBit));
    } while (llAtomicCAS(&(a->array[EEX_LL_BV_WORD(setBit)]), bv_word_old, bv_word_new));
}

uint32_t  llbvState(ll_bit_vector_t *a, uint32_t whichBit) {
    REQUIRE (set_bit <= a->size);
    return (a->array[EEX_LL_BV_WORD(whichBit)] & (1 << EEX_LL_BV_BIT(whichBit)));
}

uint32_t  llbvFF1(ll_bit_vector_t *a) {
    int i, loc;

    i = 0;
    while (i < a->size) {                   // find first nonzero word
        if (a->array[i]) { break; }
        ++i;
    }
    if (i == a->size) {                     // all words are zero, no bits set
        return (0);
    }

    loc = 32 * (a->size - 1 - i);           // location of lsb - 1 of first nonzero word
    loc += (32 - _llwordCLZ(a->array[i]));  // location of first one in word
    return (loc);
}

void  llbvAND(ll_bit_vector_t *rslt, ll_bit_vector_t *a, ll_bit_vector_t *b) {
    for (int i=0; i<EEX_CFG_THREAD_BITMAP_ARRAY_SIZ; ++i) {
        rslt->array[i] = a->array[i] & b->array[i];
    }
}

void  llbvOR(ll_bit_vector_t *rslt, ll_bit_vector_t *a, ll_bit_vector_t *b) {
    for (int i=0; i<EEX_CFG_THREAD_BITMAP_ARRAY_SIZ; ++i) {
        rslt->array[i] = a->array[i] | b->array[i];
    }
}

void  llbvCopy(ll_bit_vector_t *copy, ll_bit_vector_t *a) {
    return (llbvOR(copy,  a,  a));
}


// ==== Queue Functions ====

bool  llqPut(ll_queue_t *queue, void * val) {
    uint32_t head, head_inc;
    do {
        head = queue->head;
        head_inc = head + 1;
        if (head_inc >= queue->size) { head_inc = 0; }    // use in place of % operator
        if (llbvState(queue->valid, head_inc)) {          // valid data in next slot?
            return (FALSE);                               // queue full
        }
    } while(llAtomicCAS(&(queue->head), head, head_inc)); // move head pointer atomically
    queue->array[head] = val;                             // put the data into the queue
    llbvSet(queue->valid, head_inc);                      // mark queue slot as valid
    return (TRUE);
}

bool  llqGet(ll_queue_t *queue, void ** val) {
    do {
        tail = queue->tail;
        tail_inc = tail + 1;
        if (tail_inc >= queue->size) { tail_inc = 0; }    // use in place of % operator
        if (!llbvState(queue->valid, tail)) {             // no valid data in slot?
            return (FALSE);                               // queue empty
        }
    } while(llAtomicCAS(&(queue->tail), tail, tail_inc)); // move tail pointer atomically
    *val = queue->array[tail];                            // fetch data from queue
    llbvClr(queue->valid, tail);                          // mark queue slot as invalid
}

