/*******************************************************************************

    utils.h -
        Defines a boolean type because of poor C99 compiler support
        Defines interrupt enable/disable macros to allow nesting.
        Defines assert() macros to support design by contract.
        Redefines assert() for an embedded system with no console or file support.
        Traps assert() during testing.
        Defines a boolean type bool along with TRUE and FALSE
        Defines macros MIN, MAX, ABS
        Defines variadic function macros

    Assertions can be disabled by defining NDEBUG.
    DbC macros can be disabled by defining NASSERT.

    DbC macros mimic Eiffel:
        REQUIRE   - test precondition
        ENSURE    - test postcondition
        INVARIANT - test invariant condition
        ALLEGE    - always perform test regardless of NASSERT or NDEBUG

    Macros should not have side effects since they may be disabled. The
    exception is ALLEGE which will always execute but will not assert if
    NASSERT or NDEBUG is defined.

    DbC macros from http://www.barrgroup.com/Embedded-Systems/How-To/Design-by-Contract-for-Embedded-Software

    UNIT TESTING WITH ASSERTIONS:
    -----------------------------
    If UNIT_TEST is defined, then assertions are trapped so that the calling
    test can evaluate whether or not the code under test asserted correctly.

    Bracket a test that may generate an assertion with the macros
    ASSERT_TRY / ASSERT_ENDTRY. The macro ASSERTION(expected)
    returns a boolean that is true if the expected assertion action
    (asserted or not) equaled the actual assertion operation.

    The macro ASSERT_INIT must be included as a statement in one and only one
    module in global module scope so that the variables it declares can be
    exported. The top-level unit test framework would be a good place to
    insert the macro.

    Example:

    #include "contract.h"   // in both the test and the unit under test
    ASSERT_INIT;            // only once in the top level test
    ASSERT_TRY;
    // some test(s)
    ASSERT_ENDTRY;
    if (!ASSERTION(expected_assertion_action)) {
        // test failure
    }

    INTERRUPT CONTROL:
    ------------------
    Use LOCK / END_LOCK to bracket critical code sections. LOCK / END_LOCK
    calls can be nested.

    LOCK will mask interrupts. END_LOCK enables interrupts if and only if they
    were enabled when the paired LOCK was called.

    VARIADIC MACROS:
    ---------------
    Enable macros to accept a variable number of arguments


    COPYRIGHT NOTICE: (c) 2014 DDPA LLC
    All Rights Reserved

 ******************************************************************************/


#ifndef _contract_H_
#define _contract_H_


#ifndef bool
    typedef int bool;
    #undef  FALSE
    #undef  false
    #define FALSE (0)
    #define false FALSE
    #undef  TRUE
    #undef  true
    #define TRUE  (!FALSE)
    #define true TRUE
#endif

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define ABS(a)   (((a)< 0) ?-(a):(a))

extern void Fault_Handler(void) __attribute__((naked));


#undef assert
#ifdef  UNIT_TEST
#include  <setjmp.h>
extern int      ut_assert_f;
extern jmp_buf  ut_assert_env;
#define ASSERT_INIT         \
int      ut_assert_f;       \
jmp_buf  ut_assert_env
#define assert(condition)     (void) ((!!(condition)) || (ut_assert_f = 1, longjmp(ut_assert_env, 1), 1))
#define ASSERT_TRY            if (!setjmp(ut_assert_env)) { ut_assert_f = 0
#define ASSERT_ENDTRY         }
#define ASSERTION(expected)   (!(expected ^ ut_assert_f))
#else
#ifndef NDEBUG
#define assert(e) ((e) ? (void)0 : (void)(Fault_Handler(),0))
#else
#define assert(ignore) ((void)0)
#endif
#endif

/*
 *  NASSERT macro disables all contract validations
 *  (assertions, preconditions, postconditions, and invariants).
 */

#ifndef NASSERT         /* NASSERT not defined -- DbC enabled */
#define REQUIRE(test)   assert(test)
#define ENSURE(test)    assert(test)
#define INVARIANT(test) assert(test)
#define ALLEGE(test)    assert(test)


#else                   /* NASSERT defined -- DbC disabled */
#define REQUIRE(ignore)   ((void)0)
#define ENSURE(ignore)    ((void)0)
#define INVARIANT(ignore) ((void)0)
#define ALLEGE(test)      ((void)(test))
#endif                  /* NASSERT */


/*
 * Static Assertions
 * http://www.drdobbs.com/compile-time-assertions/184401873?pgno=1
 */
#define SA_PASTER(a)        assert_static##a
#define SA_EVALUATOR(a)     SA_PASTER(a)
#define STATIC_ASSERT(e)    enum { SA_EVALUATOR(__COUNTER__) = 1/(e) }


/*
 * Interrupt control macros
 *
 * For enable/disable interrupts use cmsis intrinsics
 * __disable_irq()
 * __enable_irq()
 */
#include <core_cmFunc.h>    // cmsis intrinsics
#define LOCK     uint32_t primask_save = __get_PRIMASK(); __disable_irq()
#define END_LOCK __set_PRIMASK(primask_save)


/*
 * Variable protection for lockless structures
 *
 * unsigned int * var_ptr is the variable (i.e. linked list pointer) to be updated
 * unsigned int   new_val is the updated (i.e. pointer) value
 * CRITICAL_VARIABLE_UPDATE returns 0 if *var_ptr successfully updated, or 1 if it was changed by another context
 *
 * Neither implementation suffers the ABA problem.
 *
 */
#if (__CORTEX_M == 0)
#define CRITICAL_VARIABLE_PROTECT(var_ptr)          (LOCK;), * (unsigned int *) var_ptr
#define CRITICAL_VARIABLE_UPDATE(var_ptr, new_val)  (* (unsigned int *) var_ptr = (unsigned int) new_val; END_LOCK;), 0
#endif

#if ((__CORTEX_M == 3) || (__CORTEX_M == 4))
#define CRITICAL_VARIABLE_PROTECT(var_ptr)          __ldrex(var_ptr)
#define CRITICAL_VARIABLE_UPDATE(var_ptr, new_val)  __strex(new_val, var_ptr)
#endif


/*
 * VARIADIC MACROS
 *
 * http://codecraft.co/2014/11/25/variadic-macros-tricks/
 *
 */

// Accept any number of args >= N, but expand to just the Nth one. In this case,
// we have settled on 6 as N. We could pick a different number by adjusting
// the count of throwaway args before N. Note that this macro is preceded by
// an underscore--it's an implementation detail, not something we expect people
// to call directly.
#define _GET_NTH_ARG(_1, _2, _3, _4, _5, N, ...) N

// Count how many args are in a variadic macro. Only works for up to N-1 args.
#define _COUNT_VARARGS(...) _GET_NTH_ARG(__VA_ARGS__, 5, 4, 3, 2, 1)

/*
 * void myVarFunc(int somevar, int num_vars, ...);
 *
 * #define MYVARFUNC(somevar, first_var, ...)   myVarFunc(somevar, _COUNT_VARARGS(__VA_ARGS__) + 1, first_var, __VA_ARGS__)
 *
 *    or
 *
 * #define MYVARFUNC(somevar, first_var, ...)   myVarFunc(somevar, first_var, __VA_ARGS__, NULL)  // NULL is the sentinal
 *
 *
 */




#endif  /* _contract_H_ */
