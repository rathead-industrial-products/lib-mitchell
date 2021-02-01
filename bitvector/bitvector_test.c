/*******************************************************************************

    Bit Vector unit test.

    COPYRIGHT NOTICE: (c) 2016 DDPA LLC
    All Rights Reserved

 ******************************************************************************/

#include  <stdbool.h>
#include  "bitvector.h"


NEW_BIT_VECTOR(bv15,  15);
NEW_BIT_VECTOR(bv15b, 15);
NEW_BIT_VECTOR(bv15c, 15);
NEW_BIT_VECTOR(bv31,  31);
NEW_BIT_VECTOR(bv32,  32);
NEW_BIT_VECTOR(bv33,  33);
NEW_BIT_VECTOR(bv129, 129);

bv_bit_vector_t * bv_tst[] = { &bv15, &bv31, &bv32, &bv33, &bv129 };

#define BV_TEST_CASES   (sizeof(bv_tst)/sizeof(bv_bit_vector_t *))

NEW_BIT_VECTOR_CONST(bvc32, 32, 0x80000001);
NEW_BIT_VECTOR_CONST(bvc64, 64, 0x8000000000000001LL);
NEW_BIT_VECTOR_CONST(bvcORDER, 64, 0x0123456789abcdefLL);



int bitvector_UNIT_TEST(void) {
    bool  pass = true;

    /* Verify bitvectors all zero */
    for (int idx=0; idx<BV_TEST_CASES; ++idx) {
        pass &= (bvFF1(bv_tst[idx]) == -1);
    }

    /* Incrementally set all bits */
    for (int idx=0; idx<BV_TEST_CASES; ++idx) {
        for (int i=0; i<bv_tst[idx]->size; ++i) {
            bvSet(bv_tst[idx], i);
            pass &= bvTest(bv_tst[idx], i);
            pass &= (bvFF1(bv_tst[idx]) == i);
        }
    }

    /* Incrementally clear all bits */
    for (int idx=0; idx<BV_TEST_CASES; ++idx) {
        for (int i=bv_tst[idx]->size-1; i>=0; --i) {
            bvClr(bv_tst[idx], i);
            pass &= !bvTest(bv_tst[idx], i);
            pass &= (bvFF1(bv_tst[idx]) == (i - 1));
        }
    }

    /* Verify Copy, OR, AND */
    bvSet(&bv15, 0);
    bvSet(&bv15, 14);
    bvCopy(&bv15b, &bv15);
    pass &= bvTest(&bv15b, 0);
    pass &= bvTest(&bv15b, 14);
    bvOR(&bv15c, &bv15, &bv15b);
    pass &= bvTest(&bv15c, 0);
    pass &= bvTest(&bv15c, 14);
    bvClr(&bv15, 0);
    bvAND(&bv15c, &bv15, &bv15b);
    pass &= !bvTest(&bv15c, 0);
    pass &=  bvTest(&bv15c, 14);

    /* Verify SetM, ClrM */
    bvSetM(&bv15, 0, 4);
    pass &= bvTest(&bv15, 0);
    pass &= bvTest(&bv15, 1);
    pass &= bvTest(&bv15, 2);
    pass &= bvTest(&bv15, 3);
    pass &= bvTest(&bv15, 4);

    bvClrM(&bv15, 4, 0);            // start and end can be in either order
    pass &= !bvTest(&bv15, 0);
    pass &= !bvTest(&bv15, 1);
    pass &= !bvTest(&bv15, 2);
    pass &= !bvTest(&bv15, 3);
    pass &= !bvTest(&bv15, 4);

    /* test return value */
    pass &= bvSet(&bv15, 4)  == 0;
    pass &= bvSet(&bv15, 4)  == 1;
    pass &= bvTest(&bv15, 4) == 1;
    pass &= bvClr(&bv15, 4)  == 1;
    pass &= bvClr(&bv15, 4)  == 0;
    pass &= bvTest(&bv15, 4) == 0;


    /* verify constant bitvectors */
    pass &= (bvFF1(&bvc32) == 31);
    pass &= bvTest(&bvc32, 0);
    pass &= (bvFF1(&bvc64) == 63);
    pass &= bvTest(&bvc64, 0);
    pass &= (bvFF1(&bvcORDER) == 56);
    pass &= bvTest(&bvcORDER, 0);
    pass &= *((uint64_t *) bvcORDER.array) == 0x0123456789abcdefLL;


    return ((int) !pass);           /* return zero if all tests pass */
}









