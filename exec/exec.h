/*******************************************************************************

    exec.h - Real time executive providing task control

    COPYRIGHT NOTICE: (c) 2011 Magomesa
    All Rights Reserved

 ******************************************************************************/


#ifndef _exec_H_
#define _exec_H_

#include <stdint.h>
#include "contract.h"


#define EXEC_TASKS_MAX                    32
#define EXEC_TASK_PRIORITY_HIGHEST        1
#define EXEC_TASK_PRIORITY_LOWEST         255
#define EXEC_TASK_PRIORITY_NON_CRITICAL   128   // leave room above and below
#define EXEC_TASK_RUN_ONCE                TRUE
#define EXEC_TASK_RUN_FOREVER             FALSE
#define EXEC_TASK_ID_ILLEGAL              (EXEC_TASKS_MAX + 1)  // cannot exceed 255


/*
 * Task priorities other than non-critical are defined here so that
 * it is clear which modules are running critical tasks.
 */
#define EXEC_TASK_PRI_ADC                 10    // process the ADC samples before doing anything else
#define EXEC_TASK_PRI_REPORT              30    // display report before printing from monitor, etc
#define EXEC_TASK_PRI_MODULE_TEST         80
#define EXEC_TASK_INSTRUMENTATION        250    // after all other tasks have run

typedef uint8_t exec_task_id_t;

/*
 * A task is defined as:
 *
 * void task(uint8_t task_id, void * param)
 *
 * where the param value is defined by, and private to, the task.
 */

void            execInit(void);
exec_task_id_t  execTaskAdd(char * name, uint8_t priority, uint16_t delay, uint16_t interval, void (*task)(uint8_t, void *), void * param, bool run_once);
void            execTaskRemove(exec_task_id_t task_id);
exec_task_id_t  execTaskExists(char * name);
void            execTick(void);
void            execSuspend(void);
void            execResume(void);
void            execRunOnce(void);
void            execRunForever(void);
exec_task_id_t  execTaskListDump(exec_task_id_t dspl_id, int (*printf) (const char *, ...));


#endif  /* _exec_H_ */
