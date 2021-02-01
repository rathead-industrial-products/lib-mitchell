/******************************************************************************

    (c) Copyright 2014 ee-quipment.com
    ALL RIGHTS RESERVED.

    monitor_cmd.c - Integrator command monitor

 *****************************************************************************/

#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include "adc.h"
#include "contract.h"
#include "memory.h"
#include "monitor.h"
#include "monitor_cmd.h"
#include "usb_serial.h"


static bool     _busy(void);
static int      _printConst(const char * const p_str, uint16_t len);
static int      _printAlloc(const char *fmt, ...);
static uint8_t  _cmdFilter(char * ip_cmd);

static int      _calibrateCmd(void);
static int      _helpCmd(void);
static int      _intervalCmd(void);
static int      _labelCmd(void);
static int      _monChgCmd(void);
static int      _pauseCmd(void);
static int      _resumeCmd(void);
static int      _scaleCmd(void);
static int      _versionCmd(void);
static int      _zeroCmd(void);



#define MON_CMD_MAX_IP_STR_LEN   32

extern char                     __VERSION_NUM_OFF[];
extern char                     __VERSION_NUM_SIZ[];
extern mon_cmd_app_data_t       ee203_mon_cmd_app_data;

static volatile bool            f_mon_cmd_busy = FALSE;
static char                     mon_cmd_ip_str[MON_CMD_MAX_IP_STR_LEN];
static uint8_t                  mon_cmd_ip_str_len = 0;
static const char               mon_cmd_prompt_str[] = "CMD> ";
static cmd_t                    mon_cmd_parsed_cmd;
static mon_cmd_action_t         mon_cmd_action[] = { { "c", _calibrateCmd },
                                                     { "h", _helpCmd },
                                                     { "i", _intervalCmd},
                                                     { "l", _labelCmd},
                                                     { "m", _monChgCmd},
                                                     { "p", _pauseCmd },
                                                     { "r", _resumeCmd },
                                                     { "s", _scaleCmd },
                                                     { "v", _versionCmd },
                                                     { "z", _zeroCmd } };

mon_obj_t  mon_cmd_obj = {
/* max chars in a cmd string */     MON_CMD_MAX_IP_STR_LEN,
/* cmd string terminating char */   '\r',
/* number of monitor commands */    sizeof(mon_cmd_action) / sizeof(mon_cmd_action_t),
/* shared monitor/app data */       &ee203_mon_cmd_app_data,
/* input command string */          mon_cmd_ip_str,
/* chars now in ip cmd str */       &mon_cmd_ip_str_len,
/* returns TRUE if mon busy */      _busy,
/* monitor prompt */                mon_cmd_prompt_str,
/* struct holding parsed cmd */     &mon_cmd_parsed_cmd,
/* monitor command filter */        _cmdFilter,
/* array of cmd:action pairs */     mon_cmd_action,
/* print a formatted string */      _printAlloc,
/* print a constant string */       _printConst,
};

static const char mon_cmd_help_message[] = "\rCOMMAND (CMD>) MONITOR COMMANDS:\r\
[H]elp                                This help message\r\
[C]alibrate                           Perform a system-level calibration\r\
[I]nterval < 1 | 10 | 100 | 1000 > ms Configure reporting interval\r\
[L]abel < [A..z] Report Header Text > Print custom label over report header\r\
[M]onitor Change                      Change from command to debug monitor\r\
[P]ause                               Stop reporting (integration continues)\r\
[R]esume                              Resume reporting\r\
[S]cale <Linear FS 10^-[val]>         Toggle SCOPE between linear and log output\r\
[V]ersion                             Display software revision number\r\
[Z]ero                                Reset cum values and time stamp to zero\r\r";

static const char mon_cmd_unrecognized_command[] = "Unrecognized Command\r";



/******************************************************************************
 *
 * bool _busy(void)
 *
 * Return TRUE if the monitor is processing a command. The f_busy flag is
 * set only if the command returns before the command is complete, e.g.
 * when calibrating or printing out large amounts of text.
 *
 *****************************************************************************/
static bool _busy(void) {
    return (f_mon_cmd_busy);
}


/******************************************************************************
 *
 * int  _printConst(const char * const p_str, uint16_t len)
 * int  _printAlloc(const char *fmt, ...)
 *
 * _printConst : Output a constant string (one with persistent storage,
 *               i.e. not an auto variable) to the USB port.
 *
 * _printAlloc : Allocate space for and output a printf style string to
 *               the USB port. The string will not be printed if a
 *               buffer cannot be allocated.
 *
 *****************************************************************************/
static int _printConst(const char * const p_str, uint16_t len) {
    REQUIRE (p_str);
    return (usbSerial_print(MON_CMD_PORT, p_str, len));
}

static int _printAlloc(const char *fmt, ...) {
    int     n_chars;
    va_list ap;

    REQUIRE (fmt);
    va_start(ap, fmt);
    n_chars = usbSerial_vprintf(MON_CMD_PORT, fmt, ap);
    va_end(ap);
    return (n_chars);
}


/******************************************************************************
 *
 * uint8_t _cmdFilter (char * ip_cmd)
 *
 * Modify the white space terminated string ip_cmd in place. Truncate to one
 * character and convert to lower-case. Replace the second character with
 * white space.
 *
 * Return the length of the resulting filtered command.
 *
 * It is an unchecked error if ip_cmd is not at least 2 characters long
 * including the terminating white space.
 *
 *****************************************************************************/
static uint8_t _cmdFilter (char * ip_cmd) {
    if ((ip_cmd[0] >= 'A') && (ip_cmd[0] <= 'Z')) {
        ip_cmd[0] += 'a' - 'A';
    }
    ip_cmd[1] = ' ';
    return (1);
}


/******************************************************************************
 *
 * Monitor Commands:
 *
 * [H]elp                                This help message
 * [C]alibrate                           Perform a system-level calibration
 * [I]nterval < 1 | 10 | 100 | 1000 > ms Configure reporting interval
 * [L]abel < [A..z] Report Header Text > Print custom label over report header
 * [M]onitor Change                      Change from command to debug monitor
 * [P]ause                               Stop reporting (integration continues)
 * [R]esume                              Resume reporting
 * [Z]ero                                Reset cum values and time stamp to zero
 *
 *  The return code tells the app what action to take.
 *
 *****************************************************************************/
static int _helpCmd(void) {
    _printConst(mon_cmd_help_message, sizeof(mon_cmd_help_message));
    return (MON_CMD_RTN_NO_ACTION);
}


static int _calibrateCmd(void) {
    _printConst("\rCalibrating, please wait...\r", sizeof("\rCalibrating, please wait...\r"));
    return (MON_CMD_RTN_CALIBRATE);
}


static int _intervalCmd(void) {
    mon_cmd_app_data_t  * p_app_data = (mon_cmd_app_data_t *) mon_cmd_obj.p_app_data;
    uint16_t intv = (uint16_t) mon_cmd_obj.p_parsed_cmd->token[1].num;

    if ((mon_cmd_obj.p_parsed_cmd->num_tokens >= 2)                   &&
        (mon_cmd_obj.p_parsed_cmd->type[1] == MON_TOKEN_NUMERIC)      &&
        ((intv == 1) || (intv == 10) || (intv == 100) || (intv == 1000))) {
        p_app_data->report_interval = intv;
        return (MON_CMD_RTN_INTERVAL);
    }
    else {
        _printConst(mon_cmd_unrecognized_command, sizeof(mon_cmd_unrecognized_command));
    }
    return (MON_CMD_RTN_NO_ACTION);
}


static int _labelCmd(void) {
    mon_cmd_app_data_t  * p_app_data = (mon_cmd_app_data_t *) mon_cmd_obj.p_app_data;
    char * label     = p_app_data->report_label;
    char * in_str    = mon_cmd_obj.p_parsed_cmd->token[1].str;
    int    label_len = 0;

    if ((mon_cmd_obj.p_parsed_cmd->num_tokens >= 2) && (mon_cmd_obj.p_parsed_cmd->type[1] == MON_TOKEN_ALPHA)) {
        while ((*in_str != '\0') && (label_len < p_app_data->report_label_len_max)) {
            *label++ = *in_str++;
            ++label_len;
        }
        p_app_data->report_label_len = label_len;
    }
    else { /* erase old label */
        p_app_data->report_label_len = 0;
    }
    return (MON_CMD_RTN_NO_ACTION);
}


static int _monChgCmd(void) {
    return (MON_CMD_RTN_CHG_TO_DBG_MON);
}


static int _pauseCmd(void) {
    return (MON_CMD_RTN_PAUSE);
}


static int _resumeCmd(void) {
    return (MON_CMD_RTN_INTERVAL);
}


static int _scaleCmd(void) {
    mon_cmd_app_data_t  * p_app_data = (mon_cmd_app_data_t *) mon_cmd_obj.p_app_data;
    uint16_t scale = (uint16_t) mon_cmd_obj.p_parsed_cmd->token[1].num;

    if ((mon_cmd_obj.p_parsed_cmd->num_tokens >= 2)                   &&
        (mon_cmd_obj.p_parsed_cmd->type[1] == MON_TOKEN_NUMERIC)      &&
        (scale <= 6)) {
        p_app_data->linear_scale = scale;
        _printAlloc("SCOPE output linear, 1V = 10^-%d A\r", scale);
    }
    else {
        p_app_data->linear_scale = ADC_VSCOPE_SCALE_LOG;
        _printConst("SCOPE output logarithmic\r", sizeof("\rSCOPE output Logarithmic\r"));
    }
    return (MON_CMD_RTN_VSCOPE_SCALE);
}


static int _versionCmd(void) {
    _printConst(__VERSION_NUM_OFF, (uint16_t) (size_t) __VERSION_NUM_SIZ);
    _printConst("\r", 1);
    return (MON_CMD_RTN_NO_ACTION);
}


static int _zeroCmd(void) {
    return (MON_CMD_RTN_RESET); //
}





