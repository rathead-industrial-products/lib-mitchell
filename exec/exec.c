/*******************************************************************************

    exec.c - Real time executive providing task control

    Code execution is serial. There are no mechanisms for pre-emption or task
    suspension, other than interrupts. To efficiently manage resource conflicts
    such as sharing a DMA channel, an executive manages all tasks on a
    cooperative, round robin basis.

    After initialization, control is passed to the executive. Control may be
    passed permanently by calling execRunForever(). This call will never
    return.

    The executive can also be called from an endless loop in main, calling
    execRunOnce() every time through the main loop. The effect is the same
    as calling execRunForever() but there may be times when there is a need
    to do something outside of the normal exec functions, such as during unit
    test when data states may need examination and/or changing outside of the
    normal program flow.

    Exec maintains a list of tasks to run. Tasks have two parameter, a
    task ID which can be used to remove the task, and a parameter cast
    to a void *. Tasks return void.

    There may be no more than EXEC_TASKS_MAX. EXEC_TASKS_MAX cannot exceed
    254. Attempting to add more than EXEC_TASKS_MAX tasks will result in a
    run-time error.

    Tasks have an 8 bit priority. Priority 1 is the highest, and 255 is the
    lowest. Zero is an illegal priority value. The task list is maintained
    in priority-sorted order. More than one task may have the same priority.

    The task list is traversed continuously if execRunForever() has been
    called, or traversed once when execRunOnce() is called. Tasks have
    a countdown timer that is decremented on every call to execTick().
    The task is called when the timer reaches zero. The timer is initially
    loaded with the delay calling parameter. After reaching zero, the timer
    is reloaded with the interval calling parameter.

    If delay == 0, the task will run as soon as it is loaded, even if it
    is a periodic task. To synchronize a new task with execTick(), set
    delay = 1 and the task will not be called until the next time execTick()
    is called and all of the periodic tasks begin execution.

    The interval value may be zero, in which case after the initial delay
    the task will be called every time the task list is traversed,
    i.e. more or less continuously.

    If all periodic tasks have not completed by the time execTick() is called
    an assert will occur, signaling a significant scheduling problem. If an
    extraordinary, long-duration event needs to be processed then execSuspend()
    can be called which will stop the scheduling check from occurring. To
    return to normal operation call execResume();

    If the run_once calling parameter is true, the task will be removed from
    the task list after it has been called once, regardless of the interval
    calling parameter. If run_once is false, the task will remain in the task
    list until explicitly removed.

    A task is added to the task list by calling execTaskAdd(). This returns
    a task id, which can be ignored, or used later to remove the task from the
    task list by calling execTaskRemove(). It is an unchecked run-time error
    to call execTaskRemove() with an invalid task id. Tasks are NOT allowed
    to be removed during interrupts as the function that removes and updates
    the task list is not interrupt-safe. EXEC_TASK_ID_ILLEGAL is guaranteed
    to always be an illegal id, useful for flagging an uninitialized or
    unused task id field.

    To determine if as task exists in the task list, and to get its
    task ID, call execTaskExists().

    There are no bounds or responsiveness guarantees from the task queue, other
    than all tasks will be called eventually in priority order. Tasks with the
    same priority may be called in any order relative to other tasks with the
    same priority.

    Exec is thread safe and reentrant.

    Example use:

    A function blocking on a resource can add a task to check if the resource
    is free. The task can be marked as run_once, and if the resource is busy,
    the task can then add itself back onto the list and continue to wait its
    turn to check the resource again. Or run_once can be false and the task
    will continue to run, and it can then be explicitly removed by calling
    execTaskRemove(task_id) once the resource is free.

    Interrupt routines can add task(s) to perform non-time-essential functions
    outside of the interrupt, reducing blockage and improving interrupt
    response times.

    COPYRIGHT NOTICE: (c) 2014 DDPA LLC
    All Rights Reserved

 ******************************************************************************/

#include  <stdint.h>
#include  <stdio.h>
#include  "contract.h"
#include  "exec.h"
#include  "printf-emb.h"



/*******************************************************************************

    Task list

    The task list is a linked list in a fixed sized static array. A separate
    linked list of empty tasks is maintained in the same array. It is a
    run-time error for the number of tasks to exceed EXEC_TASKS_MAX.

    Tasks can be added during interrupt routines, so the list management
    must be thread-safe.

 ******************************************************************************/

#define EXEC_EOL 255     // end-of-list marker

struct task_list_t {
    char *          name;
    uint16_t        interval;
    uint16_t        timer;
    void            (*task)(exec_task_id_t task_id, void *);
    void *          param;
    uint8_t         priority;
    bool            f_run_once;
    bool            f_remove;
    exec_task_id_t  next;
};

typedef struct exec_obj_t *  p_exec_obj_t;
struct exec_obj_t {
    struct  task_list_t tl[EXEC_TASKS_MAX];
    bool            f_new_tick;
    uint8_t         task_head;
    uint8_t         add_head;
    uint8_t         empty_head;
};

static struct exec_obj_t  exec_obj;
static p_exec_obj_t const p_exec_obj = &exec_obj;
static bool               exec_suspend        = FALSE;
static bool               exec_resume_pending = FALSE;

/*
 * Monitor the percent of a 1 ms tick has been used when the last task completes.
 */
#if INSTRUMENTED
#include  "instrumentation.h"
inst_param_monitor_t exec_inst_cpu_utilization = { "cpu_utilization", 0, INT32_MAX, 0, 0, 0 };
#endif



/***** Private functions *****/
static void _execTaskAdd(void);
static void _execTaskRemove(void);


/*******************************************************************************

    void execInit(void)

    Initialize the exec_obj.

 ******************************************************************************/
void  execInit(void) {

    p_exec_obj->f_new_tick  = false;
    p_exec_obj->task_head   = EXEC_EOL;
    p_exec_obj->add_head    = EXEC_EOL;
    p_exec_obj->empty_head  = 0;

    // construct linked list of empty tasks
    for (uint8_t i=0; i<EXEC_TASKS_MAX; ++i) {
        p_exec_obj->tl[i].next = i + 1;
    }
    p_exec_obj->tl[EXEC_TASKS_MAX - 1].next = EXEC_EOL;
}


/*******************************************************************************

    void  execRunOnce(void)
    void  execRunForever(void)

    Traverse the task list. This function is called by main() after
    all initialization has been performed. RunOnce() will traverse
    the list once and return. RunForever will continuously traverse
    the list and will not return.

    If new_tick is set, all timer values will be decremented before the
    traversal of the task list begins.

    On traversal of the task list if the timer has expired (== 0), the task
    is executed. The timer is reset to the interval value, which may be
    zero. If run_once is set, the task is removed from the list. If
    interval > 1 and (rtc.ms % interval) != 0 then the task execution
    is delayed for one tick. This ensures that tasks operating on different
    nodes are all synchronized.

    The amount of time remaining after all tasks have completed once is
    recorded. This time as a percentage of a complete tick is used as a
    proxy for processor utilization. There may be continuous tasks that
    keep running but that execution time is not counted toward utilization.

 ******************************************************************************/
void  execRunForever(void) { for(;;) { execRunOnce(); } }
void  execRunOnce(void) {
    exec_task_id_t  id;
    bool            f_run_all_tasks = FALSE;

    _execTaskAdd();

    if (p_exec_obj->f_new_tick) { // decrement timers on all tasks
        f_run_all_tasks = TRUE;   // record time to do all tasks once
        id = p_exec_obj->task_head;
        while (id != EXEC_EOL) {
            if (p_exec_obj->tl[id].timer) { --p_exec_obj->tl[id].timer; }
            id = p_exec_obj->tl[id].next;
        }
        p_exec_obj->f_new_tick = false;
    }

    id = p_exec_obj->task_head;
    while (id != EXEC_EOL) {
        if (p_exec_obj->tl[id].timer == 0) {
            p_exec_obj->tl[id].timer = p_exec_obj->tl[id].interval;   // reload the timer
            (*p_exec_obj->tl[id].task)(id, p_exec_obj->tl[id].param);
            if (p_exec_obj->tl[id].f_run_once) {
                p_exec_obj->tl[id].f_remove = true;
            }
        }
        id = p_exec_obj->tl[id].next;
    }
    _execTaskRemove();

    #if INSTRUMENTED
    if (f_run_all_tasks) {
        int32_t cpu_utilization = 100 - SYST_CVR_TO_PERCENT(systReadCVR());
        instUpdateMonitor(&exec_inst_cpu_utilization, cpu_utilization, systTickMS());
    }
    #endif
}


/*******************************************************************************

    exec_task_id_t  execTaskAdd(char * name, uint8_t priority, uint16_t delay, uint16_t interval, void (*task)(void *), void * param, bool run_once)

    Add a task to the task list. It is a run-time error if the number of
    tasks exceed EXEC_TASKS_MAX.

    To avoid concurrency conflicts, tasks are not added to the task list
    immediately but are put onto the add list and are added on the next
    traversal of the task list.


 ******************************************************************************/
exec_task_id_t  execTaskAdd(char * name, uint8_t priority, uint16_t delay, uint16_t interval, void (*task)(uint8_t, void *), void * param, bool run_once) {
    exec_task_id_t    id;

    LOCK;
    id = p_exec_obj->empty_head;
    assert(id != EXEC_EOL);
    p_exec_obj->empty_head  = p_exec_obj->tl[id].next;
    p_exec_obj->tl[id].next = p_exec_obj->add_head;
    p_exec_obj->add_head    = id;
    END_LOCK;

    p_exec_obj->tl[id].name       = name;
    p_exec_obj->tl[id].interval   = interval;
    p_exec_obj->tl[id].timer      = delay;
    p_exec_obj->tl[id].task       = task;
    p_exec_obj->tl[id].param      = param;
    p_exec_obj->tl[id].priority   = priority;
    p_exec_obj->tl[id].f_run_once = run_once;
    p_exec_obj->tl[id].f_remove   = false;
    return (id);
}

static void _execTaskAdd(void) {
    exec_task_id_t *  p_task_id;
    exec_task_id_t    add_task_id;

    while (p_exec_obj->add_head != EXEC_EOL) {
        LOCK;
        add_task_id = p_exec_obj->add_head;
        p_exec_obj->add_head = p_exec_obj->tl[add_task_id].next;
        END_LOCK;
        p_task_id = &(p_exec_obj->task_head);
        while ((*p_task_id != EXEC_EOL) && (p_exec_obj->tl[add_task_id].priority >= p_exec_obj->tl[*p_task_id].priority)) {
            p_task_id = &(p_exec_obj->tl[*p_task_id].next);
        }
        p_exec_obj->tl[add_task_id].next = *p_task_id;
        *p_task_id = add_task_id;
    }
}


/*******************************************************************************

    void  execTaskRemove(exec_task_id_t task_id)

    Task is removed from the periodic task list. It is an unchecked
    run-time error for task_id to be invalid. It is a checked run-time
    error to try to remove a task from an empty list.

    Tasks are only flagged for removal when they are executing (i.e. they
    remove themselves. There is no overlap with the task removal function,
    so it does not have to be interrupt-safe (no LOCK/END_LOCK).

    To avoid concurrency conflicts, tasks are not removed immediately but
    are marked and removed on the next traversal of the task list.

 ******************************************************************************/
void execTaskRemove(exec_task_id_t remove_id) {
    assert(p_exec_obj->task_head != EXEC_EOL);
    assert(remove_id < EXEC_TASKS_MAX);
    p_exec_obj->tl[remove_id].f_remove = true;
}

static void  _execTaskRemove(void) {
    exec_task_id_t *  p_id;
    exec_task_id_t *  p_next_id;
    exec_task_id_t    remove_id;

    p_id = &(p_exec_obj->task_head);
    while (*p_id != EXEC_EOL) {
        p_next_id = &(p_exec_obj->tl[*p_id].next);
        if (p_exec_obj->tl[*p_id].f_remove) {
            remove_id = *p_id;
            *p_id = *p_next_id;
            LOCK;
            *p_next_id = p_exec_obj->empty_head;
            p_exec_obj->empty_head = remove_id;
            END_LOCK;
        }
        else {
            p_id = p_next_id;
        }
    }
}


/*******************************************************************************

    exec_task_id_t execTaskExists(char * name)

    Return the TASK ID if a task with the requested name can be found in the
    task list otherwise return EXEC_TASK_ID_ILLEGAL;

    Names are only compared up to EXEC_TASK_NAME_LEN_COMPARE_MAX characters
    to avoid a buffer overflow if the terminating NUL is missing from name.

 ******************************************************************************/
#define EXEC_TASK_NAME_LEN_COMPARE_MAX    16
exec_task_id_t  execTaskExists(char * name) {
    exec_task_id_t  id;
    bool            name_equal = FALSE;
    int             len;
    char            *a, *b;

    REQUIRE (name != NULL);

    id = p_exec_obj->task_head;
    while (id != EXEC_EOL) {
        len = EXEC_TASK_NAME_LEN_COMPARE_MAX;
        name_equal = TRUE;
        a = name;
        b = p_exec_obj->tl[id].name;
        while (len-- && *a && *b) {
            if ((*a++ != *b++)) {
                name_equal = FALSE;
            }
        }
        if (name_equal) { break; }
        id = p_exec_obj->tl[id].next;
    }
    if (name_equal) { return (id); }
    else            { return (EXEC_TASK_ID_ILLEGAL); }
}


/*******************************************************************************

    void  execTick(void)

    Set a flag to cause exec to once again execute all of the timed tasks.
    This function is called once every loop update period.

    If execTick() is called while new_tick is already set this means
    the task interval counters were not even decremented before the next
    loop update cycle, indicating a severe problem. This is a
    checked run-time error.

    If exec_suspend is true, do not check f_new_tick and do not reset it.

 ******************************************************************************/

void  execTick(void) {
    if (!exec_suspend) {
        REQUIRE (p_exec_obj->f_new_tick == false);
        p_exec_obj->f_new_tick = true;
    }
    if (exec_resume_pending) {
        REQUIRE (exec_suspend);
        exec_resume_pending = FALSE;
        exec_suspend = FALSE;
    }
}


/*******************************************************************************

    void  execSuspend(void)

    Stop checking for a schedule overflow to allow something else in the
    system to consume substantial CPU time without faulting exec.

 ******************************************************************************/

void  execSuspend(void) {
    exec_suspend = TRUE;
}


/*******************************************************************************

    void  execResume(void)

    Flag execTick to resume overflow checking after the next time it is
    called. Do not enable checking immediately in order to avoid a fault
    where execTick is called immediately after execResume.

 ******************************************************************************/

void  execResume(void) {
    exec_resume_pending = TRUE;
}


/*******************************************************************************

    exec_task_id_t  execTaskListDump(exec_task_id_t dspl_id, int (*printf) (const char *, ...))

    Print the contents of the task specified by dspl_id using the supplied
    printf function. If the id is invalid display the first task in the task list.
    Return the id of the next task in the list after the one displayed.
    Return EXEC_TASK_ID_ILLEGAL after displaying the last task in the list.

 ******************************************************************************/
exec_task_id_t  execTaskListDump(exec_task_id_t dspl_id, int (*printf) (const char *, ...)) {
    struct task_list_t  task;
    exec_task_id_t      next_id;

    if (dspl_id >= EXEC_TASKS_MAX) {
        printf("Invalid Task ID number\r");
        dspl_id = p_exec_obj->task_head;
    }
    task = p_exec_obj->tl[dspl_id];

    printf("ID\tNAME\t\tINTV\tTMR\tTASK\tPARAM\tPri\tONCE\tREM\tNXT\r");
    printf("%d\t%-16s%d\t%d\t%p\t%d\t%d", dspl_id, task.name, task.interval, task.timer, task.task, task.param, task.priority);
    printf("\t%d\t%d\t%d\r", task.f_run_once ? 1 : 0, task.f_remove ? 1 : 0, task.next);

    next_id = task.next;
    return (next_id);
}



#ifdef UNIT_TEST
/******************************************************************************

    Unit test for exec.c

    Return 0 if all tests passed, 1 if there were failures.

 *****************************************************************************/

#include  <stdio.h>
#include  "contract.h"
#include  "exec.h"


/***** Prototypes *****/
int exec_UNIT_TEST(void);


/***** Private functions *****/
static void     nullTask(uint8_t unused, void * p_unused);
static void     countingTask(uint8_t unused, void * priority);
static uint8_t  taskIndex(uint16_t interval);
static void     clearExecuted(void);
static void     rtcTick(void);


/*
 * The length of the test, TOTAL_TICKS, may be slightly different than
 * TOTAL_TICKS_DESIRED due to the granularity (ticks/loop) of the test
 * loop and TOTAL_TICKS must be a multiple of 2 * TIMER_TICKS_PER_LOOP
 * so that the continuous_intermittent task gets counted properly.
 */

#define TOTAL_TICKS_DESIRED   14000       // must be >= RUN_CONT_TASK_DLY_ACTUAL to test the RUN_FOREVER continuous task
#define TIMER_TICKS_PER_LOOP  5
#define EXEC_CALLS_PER_LOOP   10
#define TOTAL_LOOPS           (2 * (TOTAL_TICKS_DESIRED/(2 * TIMER_TICKS_PER_LOOP)))
#define TOTAL_TICKS           (TOTAL_LOOPS * TIMER_TICKS_PER_LOOP)

#define PERIODIC_TASKS              6
#define RUN_CONT_TASK_DLY_DESIRED   123    // make it a multiple of EXEC_CALLS_PER_LOOP for auto-computation of calls
#define RUN_CONT_TASK_DLY_ACTUAL    (EXEC_CALLS_PER_LOOP * (RUN_CONT_TASK_DLY_DESIRED / EXEC_CALLS_PER_LOOP) + 1)   // +1 so it starts at the top of the loop


// function call counts
#define TOTAL_RUN_ONCE                  1
#define TEST_RUN_CONTINUOUS_F           ((TOTAL_TICKS > RUN_CONT_TASK_DLY_ACTUAL) ? 1 : 0)
#define TOTAL_RUN_CONTINUOUS            ((TEST_RUN_CONTINUOUS_F * (EXEC_CALLS_PER_LOOP / TIMER_TICKS_PER_LOOP) * (TOTAL_TICKS - (RUN_CONT_TASK_DLY_ACTUAL))))
#define TOTAL_PERIODIC(interval)        (TOTAL_TICKS/interval)
#define TOTAL_INTERMITTENT_CONTINUOUS   ((TOTAL_PERIODIC(TIMER_TICKS_PER_LOOP)/2) * EXEC_CALLS_PER_LOOP)
#define SUBTOTAL_TASK_CALLS_WO_PERIODIC (TOTAL_RUN_ONCE + TOTAL_RUN_CONTINUOUS + TOTAL_INTERMITTENT_CONTINUOUS + 2 /* don't know why the 2 */)

const uint16_t  task_interval[PERIODIC_TASKS] = { 10, 30, TIMER_TICKS_PER_LOOP, 70, 11, 13 };       // periodic, cannot be zero
const uint8_t   task_priority[PERIODIC_TASKS] = { EXEC_TASK_PRIORITY_NON_CRITICAL, 5, 4, 3, 2, 1 }; // must monotonically decrease

static bool task_executed[PERIODIC_TASKS] = { false, false, false, false, false, false };
static bool intermittent_continuous_enabled = false;
static exec_task_id_t intermittent_continuous_task_id;

static int  failures   = 0;
static int  task_calls = 0;


/******************************************************************************

    void execUnittest(void)

    Fill up the task queue, verify an assertion when it overflows.
    Empty the task queue, verify an assertion when it underflows.

    Reset the exec.
    Add periodic tasks of various intervals. Add continuous tasks.
    Loop for a while.
    Confirm that when each task executes that it does so in priority
    order and is correctly synchronized with the rtc. The real rtc is
    interrupt driven so it is simulated here.

    At completion, verify that the number of tasks executed was correct.

    The continuous tasks must have a priority of EXEC_TASK_PRIORITY_NON_CRITICAL
    to enable priority checking. This is a limitation of the test,
    NOT a limitation of exec.

 *****************************************************************************/
int exec_UNIT_TEST(void) {
    exec_task_id_t  id;
    int             expected_count;

    /*
     * Test execTaskAdd()
     * Fill the task queue with non-linear priorities
     */
    ASSERT_TRY;
        execInit();
        for (uint8_t i=0; i<EXEC_TASKS_MAX; ++i) {
            id = execTaskAdd("exec_test_add_success", (i + 1) % 7, 1, 1, nullTask, NULL, EXEC_TASK_RUN_FOREVER);
            execRunOnce();
            failures += (id != i) ? 1 : 0;
        }
    ASSERT_ENDTRY;
    failures += (ASSERTION(false)) ? 0 : 1;
    /*
     * Try to add one more task - it should assert
     */
    ASSERT_TRY;
        (void) execTaskAdd("exec_test_add_fail", EXEC_TASK_PRIORITY_NON_CRITICAL, 1, 1, nullTask, NULL, EXEC_TASK_RUN_FOREVER);
        execRunOnce();
    ASSERT_ENDTRY;
    failures += (ASSERTION(true)) ? 0 : 1;

    /*
     * Test execTaskRemove
     */
    ASSERT_TRY;
        for (uint8_t i=0; i<EXEC_TASKS_MAX; ++i) {
            execTaskRemove(i);
            execRunOnce();
        }
    ASSERT_ENDTRY;
    failures += (ASSERTION(false)) ? 0 : 1;
    /*
     * Try to remove one more (nonexistent) task - it should assert
     */
    ASSERT_TRY;
        execTaskRemove(0);
        execRunOnce();
    ASSERT_ENDTRY;
    failures += (ASSERTION(true)) ? 0 : 1;

    /*
     * Generate some periodic tasks at various intervals, and some continuous
     * tasks. None of these tasks should generate an assertion.
     */
    ASSERT_TRY;
        execInit();

        for (int i=0; i<PERIODIC_TASKS; ++i) {
            (void) execTaskAdd("exec_test_periodic", task_priority[i], 1, task_interval[i], countingTask, (void *) (uint32_t) task_interval[i], EXEC_TASK_RUN_FOREVER);
        }
        (void) execTaskAdd("exec_test_cont_frvr", EXEC_TASK_PRIORITY_NON_CRITICAL, RUN_CONT_TASK_DLY_ACTUAL, 0, countingTask, (void *) 0, EXEC_TASK_RUN_FOREVER);
        (void) execTaskAdd("exec_test_cont_once", EXEC_TASK_PRIORITY_NON_CRITICAL, 0, 0, countingTask, (void *) 0, EXEC_TASK_RUN_ONCE);

        int loops = TOTAL_LOOPS;
        do {
            rtcTick();
            execTick();
            execRunOnce();
            clearExecuted();

            rtcTick();
            execTick();
            execRunOnce();
            clearExecuted();
            execRunOnce();
            clearExecuted();

            rtcTick();
            execTick();
            execRunOnce();
            clearExecuted();

            rtcTick();
            execTick();
            execRunOnce();
            clearExecuted();

            rtcTick();
            execTick();
            execRunOnce();
            clearExecuted();
            execRunOnce();
            clearExecuted();
            execRunOnce();
            clearExecuted();
            execRunOnce();
            clearExecuted();
            execRunOnce();
            clearExecuted();
        } while (--loops);
    ASSERT_ENDTRY;
    failures += (ASSERTION(false)) ? 0 : 1;

    expected_count = SUBTOTAL_TASK_CALLS_WO_PERIODIC;   //lint !e506 Constant value Boolean
    for (int i=0; i<PERIODIC_TASKS; ++i) {
        expected_count += TOTAL_PERIODIC(task_interval[i]);
    }

    failures += (task_calls != expected_count) ? 1 : 0;

    return (failures);
}


/******************************************************************************

    void countingTask(uint8_t unused, void * interval)

    Task called to verify proper priority sequence and synchronization of
    the rtc.

 *****************************************************************************/
static void countingTask(uint8_t unused, void * interval) {
    uint8_t index;

    ++task_calls;

    if (interval) { // all continuous tasks MUST be EXEC_TASK_PRIORITY_NON_CRITICAL
        index = taskIndex((uint16_t) (uint32_t) interval);
        task_executed[index] = true;
        if (((uint16_t) (uint32_t) interval) == TIMER_TICKS_PER_LOOP) {
            if (intermittent_continuous_enabled) {
                execTaskRemove(intermittent_continuous_task_id);
                intermittent_continuous_enabled = false;
            }
            else {
                intermittent_continuous_task_id = execTaskAdd("exec_test_cont_int", EXEC_TASK_PRIORITY_NON_CRITICAL,
                                                  0, 0, countingTask, 0, EXEC_TASK_RUN_FOREVER);
                intermittent_continuous_enabled = true;
            }
        }
        // confirm no lower priority tasks have executed
        for (; index>0; --index) {
            failures += (task_executed[index-1]) ? 1 : 0;
        }
        // confirm task interval is synchronized to the rtc
        failures += (rtc_time % (uint16_t) (uint32_t) interval) ? 1 : 0;
    }
}


/******************************************************************************

    uint8_t  taskIndex(uint16_t interval)

    Return the index of a task in the task array identified by its unique interval.

 *****************************************************************************/
static uint8_t  taskIndex(uint16_t interval) {
    uint8_t i = 0;
    while (interval != task_interval[i]) { ++i; }
    return (i);
}


/******************************************************************************

    void clearExecuted(void)

    Clear the list of executed tasks used to verify proper priority sequencing.

 *****************************************************************************/
static void clearExecuted(void) {
    for (int i=0; i<PERIODIC_TASKS; ++i) {
        task_executed[i] = false;
    }
}


/******************************************************************************

    void nullTask(uint8_t unused, void * p_unused)

    A dummy task for the task queue fill and empty tests.

 *****************************************************************************/
static void nullTask(uint8_t unused, void * p_unused) {
}


/******************************************************************************

    void rtcTick(void)

    RTC simulation.

 *****************************************************************************/
static void rtcTick(void) {
    ++rtc_time;
}

#endif  /* UNIT_TEST */
