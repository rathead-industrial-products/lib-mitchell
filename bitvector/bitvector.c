/**
 *  \file  bitvector.c
 *  \brief A thread-safe arbitrary length set of bits.
 *
 *
 *  (c) Copyright 2016 DDPA LLC
 *  ALL RIGHTS RESERVED.
 *
 */


#include <stdint.h>
#include "bitvector.h"
#include "cpu.h"
#include "dbc.h"



#define BV_WORD(bit)      ((bit) / 32)                    ///< the word containing bit in a bitmap of size > 32
#define BV_BIT(bit)       (bit - (32 * BV_WORD(bit)))     ///< the bit position within a word


int bvSet(bv_bit_vector_t * const a, int const pos) {
    uint32_t  bv_word_old, bv_word_new;
    REQUIRE (pos >= 0);
    REQUIRE (pos <  a->size);
    do {
        bv_word_old = a->array[BV_WORD(pos)];
        bv_word_new = bv_word_old | (1 << BV_BIT(pos));
    } while (cpuCAS(&(a->array[BV_WORD(pos)]), bv_word_old, bv_word_new));
    return ((bv_word_old >> BV_BIT(pos)) & 0x0001);
}

int bvClr(bv_bit_vector_t * const a, int const pos) {
    uint32_t  bv_word_old, bv_word_new;
    REQUIRE (pos >= 0);
    REQUIRE (pos <  a->size);
    do {
        bv_word_old = a->array[BV_WORD(pos)];
        bv_word_new = bv_word_old & ~(1 << BV_BIT(pos));
    } while (cpuCAS(&(a->array[BV_WORD(pos)]), bv_word_old, bv_word_new));
    return ((bv_word_old >> BV_BIT(pos)) & 0x0001);
}

void bvSetM(bv_bit_vector_t * const a, int start, int end) {
    int temp = start;
    if (end < start) {
        start = end;
        end   = temp;
    }
    for (; start<=end; ++start) {
        (void) bvSet(a, start);
    }
}

void bvClrM(bv_bit_vector_t * const a, int start, int end) {
    int temp = start;
    if (end < start) {
        start = end;
        end   = temp;
    }
    for (; start<=end; ++start) {
        (void) bvClr(a, start);
    }
}

int bvTest(bv_bit_vector_t * const a, int const pos) {
    REQUIRE (pos >= 0);
    REQUIRE (pos <  a->size);
    return ((a->array[BV_WORD(pos)] >> BV_BIT(pos)) & 0x0001);
}

int bvFF1(bv_bit_vector_t * const a) {
    int i, loc;

    i = BV_WORD(a->size-1);
    while (i > 0) {                       // find first nonzero word, or least significant word
        if (a->array[i]) { break; }
        --i;
    }
    loc =  (32 * i) - 1;                  // location of lsb - 1 of first nonzero word
    loc += (32 - cpuCLZ(a->array[i]));    // location of first one in word
    return (loc);
}

void bvAND(bv_bit_vector_t * const rslt, bv_bit_vector_t *const a, bv_bit_vector_t * const b) {
    for (int i=0; i<=BV_WORD(rslt->size-1); ++i) {
        rslt->array[i] = a->array[i] & b->array[i];
    }
}

void bvOR(bv_bit_vector_t * const rslt, bv_bit_vector_t *const a, bv_bit_vector_t * const b) {
    for (int i=0; i<=BV_WORD(rslt->size-1); ++i) {
        rslt->array[i] = a->array[i] | b->array[i];
    }
}

void bvNOT(bv_bit_vector_t * const rslt, bv_bit_vector_t *const a) {
    for (int i=0; i<=BV_WORD(rslt->size-1); ++i) {
        rslt->array[i] = ~(a->array[i]);
    }
}

void bvCopy(bv_bit_vector_t * const copy, bv_bit_vector_t * const a) {
    bvOR(copy,  a,  a);
}


