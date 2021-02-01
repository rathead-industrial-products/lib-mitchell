/*******************************************************************************

    continuation.h -

    Macros to support continuations in C to enable stackless co-routines.

    CONT_RESUME must be the first statement in a function.

    CONT_SUSPEND returns without a value. On the next call to the function
    execution will continue immediately after the CONT_SUSPEND macro.

    CONT_YIELD(rtn_val) is identical to COND_SUSPEND but returns
    a value from the function.

    CONT_BLOCK(condition) will return every time the function is called
    as long as condition is true. When condition is false execution will
    continue immediately after the macro.

    Variables must be declared static in order to be retained across calls
    to the continuation macros.

    Example:
    int fn_generator_0_to_5_to_1(void) {
        static int x = 0;
        CONT_RESUME;
        while (x <= 5) {
            CONT_YIELD(x++);
        }
        for (x=4; x>0; --x) {
            CONT_YIELD(x);
        }
        x = 0;
    }

    Inspired by Adam Dunkels / lc-addrlabels.h

    COPYRIGHT NOTICE: (c) 2014 DDPA LLC
    All Rights Reserved

 ******************************************************************************/


#ifndef _continuation_H_
#define _continuation_H_

#include <stddef.h>


#define CONT_RESUME                   \
    static void * saved_pc = NULL;    \
    do {                              \
        void * jmp_pc = saved_pc;     \
        saved_pc = NULL;              \
        if(jmp_pc) {                  \
            goto *jmp_pc;             \
        }                             \
    } while(0)


#define CONT_SUSPEND                  \
    do {                              \
        __label__ yield_pt;           \
        saved_pc = &&yield_pt;        \
        return;                       \
        yield_pt: ;                   \
    } while (0)


#define CONT_YIELD(rtn_val)           \
    do {                              \
        __label__ yield_pt;           \
        saved_pc = &&yield_pt;        \
        return ((rtn_val));           \
        yield_pt: ;                   \
    } while (0)


#define CONT_BLOCK(condition)         \
    do {                              \
        __label__ yield_pt;           \
        saved_pc = &&yield_pt;        \
        return;                       \
        yield_pt: ;                   \
    } while (condition)



#endif  /* _continuation_H_ */
