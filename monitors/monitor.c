/******************************************************************************

    (c) Copyright 2014 ee-quipment.com
    ALL RIGHTS RESERVED.

    monitor.c - Object-based console-based monitor framework.

    An object based framework for handling a character-based console monitor
    is implemented here. The specific behavior of the monitor is contained
    in a "monitor object" that is passed to the monitor when a function is
    called.

    A command string is parsed into tokens, which are either numeric or
    non-numeric (alpha). A numeric value is one that can be represented
    by an int32_t, and may be either decimal or hex.

    decimal = [ "+" | "-" ], 0..9, { 0..9 }
    hex     = [ "+" | "-" ], "0", "x" | "X", 0..9 | a..f | A..F, { 0..9 | a..f | A..F }
    numeric = decimal | hex
    alpha   = !numeric


 *****************************************************************************/


#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "contract.h"
#include "monitor.h"



/******************************************************************************
 *
 * bool _eos(char c)
 *
 * Return TRUE if c == '\0'
 *
 *****************************************************************************/
static bool _eos(char c) {
    return (c == '\0');
}


/******************************************************************************
 *
 * bool _ws(char c)
 *
 * Return TRUE if c is a whitespace character. Whitespace is defined as any
 * character except +, -, 0..9, a..z, or A..Z.
 *
 *****************************************************************************/
static bool _ws(char c) {
    bool not_ws = FALSE;

    not_ws = not_ws || (c == '+') || (c == '-');
    not_ws = not_ws || (c >= '0') && (c <= '9');
    not_ws = not_ws || (c >= 'a') && (c <= 'z');
    not_ws = not_ws || (c >= 'A') && (c <= 'Z');

    return (!not_ws);
}


/******************************************************************************
 *
 * bool _signed(char c)
 *
 * Return TRUE if c is + or -.
 *
 *****************************************************************************/
static bool _signed(char c) {
    return ((c == '+') || (c == '-'));
}


/******************************************************************************
 * bool _decimalDigit(char c)
 *
 * Return TRUE if c is 0..9.
 *****************************************************************************/
static bool _decimalDigit(char c) {
    return (c >= '0') && (c <= '9');
}


/******************************************************************************
 *
 * bool _hexDigit(char c)
 *
 * Return TRUE if c is a hexidecimal character 0..f.
 *
 *****************************************************************************/
static bool _hexDigit(char c) {
    return (_decimalDigit(c) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F')));
}


/******************************************************************************
 *
 * bool _hexPrefix(char * str)
 *
 * Return TRUE if the first two characters of str are "0x"
 *
 *****************************************************************************/
static bool _hexPrefix(const char * str) {
    return (*str == '0' && (*(str+1) == 'x' || *(str+1) == 'X'));
}


/******************************************************************************
 *
 * uint32_t  _numericValue(char c)
 *
 * Return the integer value of c, where c is the hexidecimal character 0..f
 * or the decimal character 0..9.
 *
 *****************************************************************************/
static uint32_t  _numericValue(char c) {
    REQUIRE (_hexDigit(c));
    if (c >= 'a') { return ((uint32_t) ((unsigned char) (c - 'a')) + 10); }
    if (c >= 'A') { return ((uint32_t) ((unsigned char) (c - 'A')) + 10); }
    return ((uint32_t) ((unsigned char) (c - '0')));
}


/******************************************************************************
 *
 * bool _numeric(char ** p_str, uint32_t * value)
 *
 * Examine the string beginning at **p_str and ending at the next white space
 * character. Modify *p_str to point to the string terminating white
 * space. If the string is numeric, set *value to the numeric value of the
 * string and return TRUE. Otherwise return FALSE. If *p_str is non-numeric
 * value will be modified and undefined.
 *
 * Hex numbers are always positive, but may be preceded by a '-' to make them
 * negative. If the magnitude of a hex value is larger than 0x8fffffff the
 * results are undefined, but almost certainly will be incorrect.
 *
 * This function may be called with **p_str == white space and will return FALSE;
 *
 *****************************************************************************/
static bool _numeric(char ** p_str, int32_t * value) {
    #define  DEC 10
    #define  HEX 16
    int32_t  mult        = DEC;
    bool     is_numeric  = FALSE;
    bool     is_negative = FALSE;
    char *   str         = *p_str;
    bool (*_digit)(char) = _decimalDigit;

    if (_ws(*str)) { return (FALSE); }

    *value = 0;

    /* optional sign */
    if (_signed(*str)) {
        if (*str == '-') { is_negative = TRUE; }
        ++str;
    }
    /* optional hex prefix */
    if (_hexPrefix(str)) {
        str += 2;
        mult = HEX;
        _digit = _hexDigit;
    }
    /* process digits until number string is complete or it can be determined that this is not a number */
    while (_digit(*str)) {
        *value = (*value * mult) + (int32_t) _numericValue(*str);
        ++str;
    }
    /* number string is complete or non-numeric character found */
    if (_ws(*str)) {
        is_numeric = TRUE;
        *value *= is_negative ? -1 : 1;
    }
    else {
        /* non numeric, move pointer to terminating white */
        while (!_ws(*str)) { ++str; }
    }
    *p_str = str;
    return (is_numeric);
}

/******************************************************************************
 *
 * void _parse(char * cmd_str)
 *
 * Parse a nul-terminated command string into tokens. Populate the static
 * variable cmd with the tokens.
 *
 *****************************************************************************/
static void _parse(char * cmd_str, cmd_t * p_cmd) {
    int32_t num_value;

    p_cmd->num_tokens = 0;
    while (!_eos(*cmd_str) && _ws(*cmd_str)) { ++cmd_str; } // skip past white space

    while (!_eos(*cmd_str) && (p_cmd->num_tokens <= MON_MAX_TOKENS)) {
        p_cmd->type[p_cmd->num_tokens] = MON_TOKEN_ALPHA;
        p_cmd->token[p_cmd->num_tokens].str = cmd_str;
        if (_numeric(&cmd_str, &num_value)) {
            p_cmd->type[p_cmd->num_tokens] = MON_TOKEN_NUMERIC;
            p_cmd->token[p_cmd->num_tokens].num = num_value;
        }
        ++(p_cmd->num_tokens);
        while (!_eos(*cmd_str) && _ws(*cmd_str)) { ++cmd_str; } // skip past white space
    }
}


/*******************************************************************************

    History Buffer

    The history buffer is a ring buffer that holds a linked list of the most
    recently executed commands. The up and down arrows are used to go
    backward/forward in history.

    The size of the history buffer is set by MON_HIST_BUF_SIZE in monitor.h.

    static bool _addHist(const char * const cmd_str, const uint16_t cmd_len)
        Add a command string to the history buffer. Return FALSE if cmd_str
        will not fit or cmd_str is null.

    static uint8_t _getHist(int event, char * const cmd_str)
        Copy the nth most recent event to cmd_str[], or the oldest if
        there were not n events. Return the length of cmd_str, or 0 if
        there is no history.

    static bool _useHist(mon_obj_t * const mon_obj, const int event)
        Copy the nth most recent command to mon_obj->cmd_str, erase the current
        command string from the console and print the new cmd_str to the
        console. Return FALSE if there is no history.

******************************************************************************/
#ifdef UNIT_TEST
#undef  MON_HIST_BUF_SIZE
#define MON_HIST_BUF_SIZE  24
#endif

#if (MON_HIST_BUF_SIZE > 256)
#error ("History buffer cannot be larger than 256 bytes.")
#endif

static struct {
    uint8_t   start;    // start of linked list, location after most recent entry
    uint8_t   end;      // end of linked list, location before oldest entry
    uint8_t   cursor;   // nth most recent last fetched command
    char      hist[MON_HIST_BUF_SIZE];
} g_mon_hist_buf = { 0 };

static void _incHistBufPtr(uint8_t *ptr) {
    *ptr += 1;
    if (*ptr >= MON_HIST_BUF_SIZE) { *ptr = 0; }
}

static void _decHistBufPtr(uint8_t *ptr) {
    if (*ptr == 0) { *ptr = MON_HIST_BUF_SIZE; }
    *ptr -= 1;
}

static bool _addHist(const char * const cmd_str, const uint16_t cmd_len) {
    uint8_t ptr, rem, wrap;

    // ignore null cmd or cmd too long to fit into history buffer
    if ((!cmd_len) || (cmd_len > MON_HIST_BUF_SIZE - 2)) { return (FALSE); }

    // drop oldest entries until new entry will fit
    while (1) {
        wrap = 0;
        if (g_mon_hist_buf.end <= g_mon_hist_buf.start) {
            wrap = MON_HIST_BUF_SIZE;   // adjust for ring buffer wrap
        }
        rem = (wrap + g_mon_hist_buf.end) - g_mon_hist_buf.start;
        if (rem >= cmd_len + 1) { break; }    // sufficient room

        // traverse linked list and drop oldest entry
        ptr = g_mon_hist_buf.start;
        while (g_mon_hist_buf.hist[ptr] != g_mon_hist_buf.end) {
            ptr = g_mon_hist_buf.hist[ptr];
        }
        g_mon_hist_buf.end = ptr;
    }

    // append command string to head of linked list
    ptr = g_mon_hist_buf.start;
    for (uint16_t i=0; i<cmd_len; ++i) {
        _incHistBufPtr(&ptr);
        g_mon_hist_buf.hist[ptr] = cmd_str[i];
    }
    _incHistBufPtr(&ptr);
    g_mon_hist_buf.hist[ptr] = g_mon_hist_buf.start;
    g_mon_hist_buf.start = ptr;

    // reset cursor
    g_mon_hist_buf.cursor = 0;

    return (TRUE);
}

static uint8_t _getHist(int event, char * const cmd_str) {
    uint8_t   ptr, hist_end, cmd_len;

    // return if history buffer is empty or event is not in the past
    if ((event <= 0) || (g_mon_hist_buf.start == g_mon_hist_buf.end)) {
        return (0);
    }

    // traverse list to find event or end-of-list
    ptr = g_mon_hist_buf.start;
    while ((ptr != g_mon_hist_buf.end) && event--) {
        hist_end = ptr;
        ptr = g_mon_hist_buf.hist[ptr];
    }

    // copy history to command string
    cmd_len = 0;
    _incHistBufPtr(&ptr);
    while (ptr != hist_end) {
        cmd_str[cmd_len++] = g_mon_hist_buf.hist[ptr];
        _incHistBufPtr(&ptr);
    };

    return (cmd_len);
}

static bool _useHist(mon_obj_t * const mon_obj, const int event) {
    char *    cmd_str;
    uint8_t   prev_cmd_str_len, hist_len;
    bool      rslt = FALSE;

    prev_cmd_str_len = *mon_obj->p_cmd_str_len;
    cmd_str = mon_obj->cmd_str;

    hist_len = _getHist(event, cmd_str);
    if (hist_len) {
        while (prev_cmd_str_len) {  // erase console command
            if (prev_cmd_str_len >= 16) {
                (void) mon_obj->printConst("\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b", 48);
                prev_cmd_str_len -= 16;
            }
            else {
                (void) mon_obj->printConst("\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b\b \b", 3 * prev_cmd_str_len);
                break;
            }
        }
        *mon_obj->p_cmd_str_len = hist_len;
        (void) mon_obj->printConst(cmd_str, hist_len);
        rslt = TRUE;
    }
    return (rslt);
}


/*******************************************************************************

    int monMonitor(mon_obj_t * mon_obj, char * p_cmd, uint16_t cmd_len)

    p_cmd is not nul terminated, cmd_len specifies the length of the string.

    Parse the command. If the command is complete then search the mon_obj
    for a matching command and call the associated function.

    The meaning of the return value is specific to the monitor object, except
    zero implies that no action has been taken.

    After passing a command to a monitor object, wait for the command to
    complete before printing the prompt.

******************************************************************************/
int monMonitor(mon_obj_t * mon_obj, const char * p_cmd, uint16_t cmd_len) {
    static bool   f_prompt_displayed = FALSE;
    int           rslt = MON_RTN_NO_ACTION;
    uint8_t       filtered_cmd_len;
    mon_action_t  action = NULL;

    if (!f_prompt_displayed && !mon_obj->busy()) {
        (void) mon_obj->printConst(mon_obj->prompt, sizeof(mon_obj->prompt));
        f_prompt_displayed = TRUE;
    }

    /* return if command string is empty */
    if (cmd_len == 0) { return (MON_RTN_NO_ACTION); }

    /* command too long, complain and ignore input */
    if ((*(mon_obj->p_cmd_str_len) + cmd_len) > mon_obj->max_cmd_str_len) {
        (void) mon_obj->printConst("\rCommand too long\r", sizeof("\rCommand too long\r"));
    }

    else {
        /* Echo the input
         * Up and down arrows replace the cmd_str with an event from the
         * history buffer. Backspace erases and cmd_term signals the command
         * is complete. Append all other printing characters to cmd_str.
         */
        for (int i=0; i<cmd_len; ++i) {
            // vt100 arrow escape sequence
            if (((cmd_len - i) >= 3) && (p_cmd[i] == 0x1b)) {             // arrow escape
                if ((p_cmd[i+1] == 0x5b) && (p_cmd[i+2] == 0x41)) {       // up
                    if (_useHist(mon_obj, g_mon_hist_buf.cursor+1)) {
                        ++g_mon_hist_buf.cursor;
                    }
                }
                else if ((p_cmd[i+1] == 0x5b) && (p_cmd[i+2] == 0x42)) {  // down
                    if (_useHist(mon_obj, g_mon_hist_buf.cursor-1)) {
                        --g_mon_hist_buf.cursor;
                    }
                }
                i += 2;                                                   // 3 character escape code
                // left and right are  ignored
            }

            // backspace
            else if (p_cmd[i] == '\b') {
                // erase by backing up, printing a space, then backing up again
                if (*(mon_obj->p_cmd_str_len) != 0) {
                    (void) mon_obj->printConst("\b \b", 3);
                    *(mon_obj->p_cmd_str_len) -= 1;
                }
            }

            // display and append chars to cmd string and/or terminate command string
            else if ((p_cmd[i] == mon_obj->cmd_term) || ((p_cmd[i] >= ' ') && (p_cmd[i] <= '~'))) {
                (void) mon_obj->printConst(&p_cmd[i], 1);
                mon_obj->cmd_str[*(mon_obj->p_cmd_str_len)] = p_cmd[i];
                *(mon_obj->p_cmd_str_len) += 1;
                if (p_cmd[i] == mon_obj->cmd_term) {
                    break;    // ignore all following characters
                }
            }
        }

        /* return and wait for more input if command has not been terminated */
        if (mon_obj->cmd_str[*(mon_obj->p_cmd_str_len)-1] != mon_obj->cmd_term) {
            return (MON_RTN_NO_ACTION);
        }

        /* command complete, replace line terminator (e.g. '\r') with string terminator '\0', add to history buffer and parse */
        mon_obj->cmd_str[*(mon_obj->p_cmd_str_len)-1] = '\0';
        _addHist(mon_obj->cmd_str, *mon_obj->p_cmd_str_len - 1);  // history doesn't need string terminator
        _parse(mon_obj->cmd_str, mon_obj->p_parsed_cmd);

        /* if there appears to be a valid command, process it, otherwise ignore */
        if (mon_obj->p_parsed_cmd->num_tokens && (mon_obj->p_parsed_cmd->type[0] == MON_TOKEN_ALPHA)) {

            /* run command through filter */
            filtered_cmd_len = mon_obj->cmdFilter(mon_obj->p_parsed_cmd->token[0].str);

            /* find command in action table */
            for (int i=0; i<mon_obj->n_cmd_actions; ++i) {
                if (0 == strncmp(mon_obj->p_parsed_cmd->token[0].str, mon_obj->cmd_action[i].cmd, filtered_cmd_len)) {
                    action = mon_obj->cmd_action[i].action;
                }
            }
            if (action) { rslt = action(); }
        }
        if (mon_obj->p_parsed_cmd->num_tokens && !action) {   // don't complain if user just hit return at prompt
            (void) mon_obj->printConst("Unrecognized Command\r", sizeof("Unrecognized Command\r"));
        }
    }

    /* clear command string in preparation for the next command, print prompt when command has been completed */
    *(mon_obj->p_cmd_str_len) = 0;
    f_prompt_displayed = FALSE;
    return (rslt);
}



#ifdef UNIT_TEST

static bool     _busyMON_UT(void) { return (FALSE); }
static uint8_t  _cmdFilterMON_UT(char * s) { return (0); }
static int      _printMON_UT(const char *s, uint16_t len) { return (0); }

#define MON_UT_CMD_STR_LEN          40
static char     mon_ut_cmd_ip_str[MON_UT_CMD_STR_LEN];
static uint8_t  mon_ut_cmd_ip_str_len = 0;
static cmd_t    mon_ut_cmd;

mon_obj_t  mon_ut_obj = {
/* max chars in a cmd string */     40,
/* cmd string terminating char */   '\r',
/* number of monitor commands */    0,
/* shared monitor/app data */       NULL,
/* input command string */          mon_ut_cmd_ip_str,
/* chars now in ip cmd str */       &mon_ut_cmd_ip_str_len,
/* returns TRUE if mon busy */      _busyMON_UT,
/* monitor prompt */                "UNIT_TEST> ",
/* struct holding parsed cmd */     &mon_ut_cmd,
/* monitor command filter */        _cmdFilterMON_UT,
/* array of cmd:action pairs */     NULL,
/* print a formatted string */      NULL,
/* print a constant string */       _printMON_UT,
};



int monitor_UNIT_TEST(void)
{
  bool  pass = TRUE;

  /* test command parser */
  _parse("cmd 1234 subcmd +0x0000 -1234 0x8fffffff -0x4d2\r", &mon_ut_cmd);
  pass &= (mon_ut_cmd.num_tokens == 7);
  pass &= (mon_ut_cmd.type[0] == MON_TOKEN_ALPHA);
  pass &= (mon_ut_cmd.type[1] == MON_TOKEN_NUMERIC);
  pass &= (mon_ut_cmd.type[2] == MON_TOKEN_ALPHA);
  pass &= (mon_ut_cmd.type[3] == MON_TOKEN_NUMERIC);
  pass &= (mon_ut_cmd.type[4] == MON_TOKEN_NUMERIC);
  pass &= (mon_ut_cmd.type[5] == MON_TOKEN_NUMERIC);
  pass &= (mon_ut_cmd.type[6] == MON_TOKEN_NUMERIC);
  pass &= (0 == strncmp(mon_ut_cmd.token[0].str, "cmd", 3));
  pass &= (0 == strncmp(mon_ut_cmd.token[2].str, "subcmd", 6));

  /* test backspace and history buffer */
  /* fill history buffer */
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "12345\r", strlen("12345\r")));                         // 12345
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "123\b4\r", strlen("123\b4\r")));                       // 124
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "12\b\b\b\bAB\r", strlen("12\b\b\b\bAB\r")));           // AB
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41""CD\r", strlen("\x1b\x5b\x41""CD\r")));   // up-arrow, C, D = ABCD
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "ab\x1b\x5b\x42\x1b\x5b\x42\r", strlen("ab\x1b\x5b\x42\x1b\x5b\x42\r"))); // a, b, dn-arrow, dn-arrow = ab

  /* verify history buffer contents */
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "GARBAGE", strlen("GARBAGE")));           // random stuff into the command buffer
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41", strlen("\x1b\x5b\x41"))); // up-arrow
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "ab", *mon_ut_obj.p_cmd_str_len));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41", strlen("\x1b\x5b\x41"))); // up-arrow
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "ABCD", *mon_ut_obj.p_cmd_str_len));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41", strlen("\x1b\x5b\x41"))); // up-arrow
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "AB", *mon_ut_obj.p_cmd_str_len));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41", strlen("\x1b\x5b\x41"))); // up-arrow
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "124", *mon_ut_obj.p_cmd_str_len));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41", strlen("\x1b\x5b\x41"))); // up-arrow
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "12345", *mon_ut_obj.p_cmd_str_len));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41", strlen("\x1b\x5b\x41"))); // up-arrow, bottoming out
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "12345", *mon_ut_obj.p_cmd_str_len));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41", strlen("\x1b\x5b\x41"))); // up-arrow, bottoming out
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "12345", *mon_ut_obj.p_cmd_str_len));

  /* test ring buffer wraparound */
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "NOW IS THE\r", strlen("NOW IS THE\r")));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "TIME FOR\r", strlen("TIME FOR\r")));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "ALL GOOD\r", strlen("ALL GOOD\r")));

  /* verify "NOW IS THE" got overwritten */
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "GARBAGE", strlen("GARBAGE")));           // random stuff into the command buffer
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41", strlen("\x1b\x5b\x41"))); // up-arrow
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "ALL GOOD", *mon_ut_obj.p_cmd_str_len));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41", strlen("\x1b\x5b\x41"))); // up-arrow
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "TIME FOR", *mon_ut_obj.p_cmd_str_len));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41", strlen("\x1b\x5b\x41"))); // up-arrow, bottoming out
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "TIME FOR", *mon_ut_obj.p_cmd_str_len));

  /* erase "TIME FOR" and fill ring buffer just to capacity */
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\b\b\b\b\b\b\b\b\r", strlen("\b\b\b\b\b\b\b\b\r"))); // all bs, enter shouldn't put anything into the history buffer
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "MEN2\r", strlen("MEN2\r")));

  /* verify */
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "GARBAGE", strlen("GARBAGE")));           // random stuff into the command buffer
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41", strlen("\x1b\x5b\x41"))); // up-arrow
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "MEN2", *mon_ut_obj.p_cmd_str_len));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41", strlen("\x1b\x5b\x41"))); // up-arrow
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "ALL GOOD", *mon_ut_obj.p_cmd_str_len));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41", strlen("\x1b\x5b\x41"))); // up-arrow
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "TIME FOR", *mon_ut_obj.p_cmd_str_len));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41", strlen("\x1b\x5b\x41"))); // up-arrow, bottoming out
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "TIME FOR", *mon_ut_obj.p_cmd_str_len));

  // fill to capacity
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\r", strlen("\r")));                         // clean command line
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "1234567890123456789012\r", strlen("1234567890123456789012\r")));   // hist buf can hold MON_HIST_BUF_SIZE - 2
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "GARBAGE", strlen("GARBAGE")));               // random stuff into the command buffer
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41", strlen("\x1b\x5b\x41")));     // up-arrow
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "1234567890123456789012", *mon_ut_obj.p_cmd_str_len));    // verify

  // verify oversized cmd lines will not get saved to the history buffer
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\r", strlen("\r")));   // clean command line
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "abcdefghijklmnopqrstuvw\r", strlen("abcdefghijklmnopqrstuvw\r")));  // hist buf cannot hold MON_HIST_BUF_SIZE - 1
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "GARBAGE", strlen("GARBAGE")));               // random stuff into the command buffer
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41", strlen("\x1b\x5b\x41")));     // up-arrow
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "1234567890123456789012", *mon_ut_obj.p_cmd_str_len));    // verify previous history was not overwritten

  // verify up and down
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\r", strlen("\r")));     // clean command line
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "1\r", strlen("1\r")));   // Nine events
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "2\r", strlen("2\r")));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "3\r", strlen("3\r")));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "4\r", strlen("4\r")));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "5\r", strlen("5\r")));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "6\r", strlen("6\r")));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "7\r", strlen("7\r")));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "8\r", strlen("8\r")));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "9\r", strlen("9\r")));
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41\x1b\x5b\x41\x1b\x5b\x41", strlen("\x1b\x5b\x41\x1b\x5b\x41\x1b\x5b\x41")));     // 3 x up-arrow
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "7", *mon_ut_obj.p_cmd_str_len));     // 3rd previous
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x41\x1b\x5b\x41\x1b\x5b\x41", strlen("\x1b\x5b\x41\x1b\x5b\x41\x1b\x5b\x41")));     // 3 x up-arrow
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "4", *mon_ut_obj.p_cmd_str_len));     // 6th previous
  pass &= (MON_RTN_NO_ACTION == monMonitor(&mon_ut_obj, "\x1b\x5b\x42\x1b\x5b\x42\x1b\x5b\x42\x1b\x5b\x42", strlen("\x1b\x5b\x42\x1b\x5b\x42\x1b\x5b\x42\x1b\x5b\x42")));     // 4 x dn-arrow
  pass &= (0 == strncmp(mon_ut_obj.cmd_str, "8", *mon_ut_obj.p_cmd_str_len));     // 6 - 4 = 2nd previous

  return (!pass);
}
#endif  /* UNIT_TEST */



