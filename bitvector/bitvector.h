
/**
 *  \file  bitvector.h
 *  \brief A thread-safe arbitrary length set of bits.
 *
 *  The position of a bit within a bit vector is numbered from 0 (least-significant bit) to size-1 (most-significant bit).
 *  32 bit words in a bit field are ordered in little-endian format, so bits 0-31 will be in the word at the lowest address
 *  and bit 32-63 will be in the word at the lowest address +4, etc.
 *
 *  Only the single-bit operations bvSet and bvClr are thread safe.
 *  The thread-safe data structure is lockless for bvSet and bvClr if a compare-and-swap primitive (CAS) is available.
 *  Arm CM3/CM4 processors implement CAS using ldrex/strex and do not disable interrupts.
 *  Arm CM0/CM0+ processors do not have an atomic operator and must briefly disable interrupts.
 *
 *  The bit vector name is globally visible.
 *
 *  Usage Example:
 *  NEW_BIT_VECTOR(name, size);
 *  int mybit = bvTest(name, whichbit);
 *  bvSet(name, 7);
 *
 *
 *  Revision History:
 *    03/11/16  Initial release
 *    08/15/16  Added bvSetM, bvClrM
 *
 *
 *  (c) Copyright 2016 DDPA LLC
 *  ALL RIGHTS RESERVED.
 *
 */


#ifndef _bitvector_H_
#define _bitvector_H_


#include <stdint.h>


/// Bit Vector.
typedef struct bv_bit_vector_t {
    uint32_t      size;       ///< Bits in vector.
    uint32_t *   array;       ///< Pointer to bit vector.
} const bv_bit_vector_t;

/// Macro to define the storage for a bit vector. name has global scope.
#define NEW_BIT_VECTOR(name, size)                                   \
uint32_t name##_array[ ((size-1)/32)+1 ] = { 0 };                    \
bv_bit_vector_t name = { size, name##_array }

/// Generates a constant Bit Vector up to 64 bits in length.
#define NEW_BIT_VECTOR_CONST(name, size, val)                        \
const uint64_t name##_array = (uint64_t) val ;                       \
bv_bit_vector_t name = { size, (uint32_t *) &name##_array }


// ==== Bit Vector Functions ====

/// Set a bit (lsb = 0). Thread safe. Operation is lockless.
/// It is a checked run time error for pos to be >= a.size or < 0.
/// \return the previous state of a[pos] (0 or 1).
int  bvSet(bv_bit_vector_t * const a, int const pos);

/// Clear a bit (lsb = 0). Thread safe. Operation is lockless.
/// It is a checked run time error for pos to be >= a.size or < 0.
/// \return the previous state of a[pos] (0 or 1).
int  bvClr(bv_bit_vector_t * const a, int const pos);

/// Set a contiguous series of bits from start to end inclusive (lsb = 0).
/// It is a checked run time error for start or end to be >= a.size or < 0.
void  bvSetM(bv_bit_vector_t * const a, int start, int end);

/// Clear a contiguous series of bits from start to end inclusive (lsb = 0).
/// It is a checked run time error for start or end to be >= a.size or < 0.
void  bvClrM(bv_bit_vector_t * const a, int start, int end);

/// Find the state of a bit (lsb = 0). It is a checked run time error for pos to be >= a.size or < 0.
/// \return the state (1 or 0) of the bit at position pos.
int   bvTest(bv_bit_vector_t * const a, int const pos);

/// Find the position of the first one in the bitmap (lsb = 0).
/// \return the position of the first one bit, or -1 if there are no ones.
int   bvFF1(bv_bit_vector_t * const a);

/// Logical AND of two bitmaps. rslt, a, and b must be the same size.
/// \return the logical AND of bit vectors a and b.
void  bvAND(bv_bit_vector_t * const rslt, bv_bit_vector_t *const a, bv_bit_vector_t * const b);

/// Logical OR of two bitmaps. rslt, a, and b must be the same size.
/// \return the logical OR of bit vectors a and b.
void  bvOR(bv_bit_vector_t * const rslt, bv_bit_vector_t *const a, bv_bit_vector_t * const b);

/// Logical NOT of a bitmaps. rslt and a must be the same size.
/// \return the logical NOT of bit vector a.
void  bvNOT(bv_bit_vector_t * const rslt, bv_bit_vector_t *const a);

/// Copy a bitmap. copy and a must be the same size.
/// \return a copy of bit vector a.
void  bvCopy(bv_bit_vector_t * const copy, bv_bit_vector_t * const a);



#endif  /* _bitvector_H_ */
