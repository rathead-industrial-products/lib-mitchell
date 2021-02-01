/*****************************************************************************
 *
    (c) Copyright 2014 ee-quipment.com
    ALL RIGHTS RESERVED.

    monitor.h - Object-based console-based monitor framework.

    An object based framework for handling a character-based console monitor
    is implemented here. The specific behavior of the monitor is contained
    in a "monitor object" that is passed to the monitor when a function is
    called.

    A monitor command is of the form <alpha_token | numeric_token> .. < > .
    There can be up to MON_MAX_TOKENS in a single command.
    A numeric_string is a signed integer that can be contained in an int32_t.

    Any monitor implementation should implement a "help" command. This will
    be called from the monitor when it cannot parse the input command string.

 *****************************************************************************/

#ifndef _monitor_H_
#define _monitor_H_

#include  <stdint.h>


#define MON_RTN_NO_ACTION     0
#define MON_MAX_TOKENS        7
#define MON_HIST_BUF_SIZE     128     // command history ring buffer size

typedef enum { MON_TOKEN_ALPHA, MON_TOKEN_NUMERIC } token_t;

typedef union {
    int32_t  num;
    char *   str;
} token_value_t;

typedef struct {
    uint8_t       num_tokens;
    token_t       type[MON_MAX_TOKENS];
    token_value_t token[MON_MAX_TOKENS];
} cmd_t;


typedef int (*mon_action_t)(void);

typedef struct {
    const char  *       cmd;
    const mon_action_t  action;
} mon_cmd_action_t;


/*
 * The behavior of the monitor is determined by a "monitor object" defined
 * in a mon_obj_t.
 *    max_cmd_str_len : The longest input string allowed. Exceeding this length will cause the input command to be ignored and cleared.
 *    cmd_term        : The command terminating character, e.g. '\r'
 *    n_cmd_actions   : The size of the cmd_action array
 *    void *          : Pointer to data shared between monitor and application - must be cast to proper type
 *    cmd_str         : The input command string, may be built up over several calls to monMonitor()
 *    p_cmd_str_len   : The number of characters currently in cmd_str
 *    busy()          : Returns TRUE if the monitor is processing a command
 *    prompt          : The command prompt string, e.g. ">"
 *    p_parsed_cmd    : The command string parsed into tokens
 *    cmdFilter()     : A function which takes a raw command, filters it in place replacing truncated chars with white space and returns the length of the filtered cmd, e.g. converts to all lower case and truncates to a single letter. (MEMSET -> m)
 *    cmd_action      : An array of { CMD : FUNCTION_CALL } pairs where the function is called when cmdFilter(input command) matches CMD.
 *    printAlloc()    : A function call that allocates a persistent buffer and prints a formatted string. Returns the number of chars to be printed. Nothing is printed if the allocation fails.
 *    printConst()    : A function call that prints a constant string to the console and returns the number of chars printed.
 */
typedef const struct {
    uint8_t                   max_cmd_str_len;
    char                      cmd_term;
    uint8_t                   n_cmd_actions;
    void *                    p_app_data;
    char *                    cmd_str;
    uint8_t *                 p_cmd_str_len;
    bool                      (*busy)(void);
    const char *              prompt;
    cmd_t *                   p_parsed_cmd;
    uint8_t                   (*cmdFilter)(char *);
    mon_cmd_action_t *        cmd_action;
    int                       (*printAlloc)(const char *fmt, ...);
    int                       (*printConst)(const char *, uint16_t);
} mon_obj_t;


int monMonitor(mon_obj_t * mon_obj, const char * p_cmd, uint16_t cmd_len);



#endif  /* _monitor_H_ */
