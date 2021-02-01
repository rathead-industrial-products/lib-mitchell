/******************************************************************************

    (c) Copyright 2014 ee-quipment.com
    ALL RIGHTS RESERVED.

    monitor_cmd.h - Integrator command monitor

 *****************************************************************************/

#ifndef _monitor_cmd_H_
#define _monitor_cmd_H_

#include <stdint.h>
#include "monitor.h"


/*
 * Return Codes
 */
#define MON_CMD_RTN_NO_ACTION       0
#define MON_CMD_RTN_RESET           1
#define MON_CMD_RTN_INTERVAL        2
#define MON_CMD_RTN_PAUSE           3
#define MON_CMD_RTN_CALIBRATE       4
#define MON_CMD_RTN_VSCOPE_SCALE    5
#define MON_CMD_RTN_CHG_TO_DBG_MON  6

#define MON_CMD_PORT                CONSOLE_CMD_PORT


extern mon_obj_t  mon_cmd_obj;


typedef struct {
    uint16_t      report_interval;
    const uint8_t report_label_len_max;
    uint8_t       report_label_len;
    char *        report_label;
    uint16_t      linear_scale;
} mon_cmd_app_data_t;


#endif  /* _monitor_cmd_H_ */
