/*******************************************************************************

    Logging Utility

    Output a message to the J-Link debuffer using the SEGGER_RTT mechanism.

    If an SRAM log buffer is defined in the linker (ld) file, then use it as
    a circular log buffer. log_flush2NV copies the circular buffer to non-volatile
    memory.

    A separate utility for outputting via the SWO port is provided.

    COPYRIGHT NOTICE: (c) 2018 DDPA LLC
    All Rights Reserved

 ******************************************************************************/

#include  <printf-emb_tiny.h>
#include  "logging.h"
#include  "segger/segger_rtt/SEGGER_RTT.h"


#define MAX_LOG_ENTRY_LEN   64    // including end-of-string NUL

extern char     __LOGFILE_START;
extern size_t   __LOGFILE_SIZE;


static char *g_eol = &__LOGFILE_START;   // end-of-log pointer

// copy n characters of string s to the logfile
static void _copy_to_logfile(char *s, int n) {
    size_t logfile_size = (size_t) (&__LOGFILE_SIZE);

    if (n < logfile_size) {
        while (n--) {
            *g_eol = *s++;
            if (++g_eol >= &__LOGFILE_START + logfile_size) {
                g_eol = &__LOGFILE_START;
            }
        };
    }
}


void log_printf(const char *fmt, ...) {
    unsigned cnt;
    char     buf[MAX_LOG_ENTRY_LEN];
    va_list  ap;

    va_start(ap, fmt);
    cnt = vsnprintf(buf, MAX_LOG_ENTRY_LEN, fmt, ap);
    va_end(ap);

    _copy_to_logfile(buf, cnt+1);  // include terminating NUL
    SEGGER_RTT_Write(0, buf, cnt);
    SEGGER_RTT_Write(0, "\n", 1);
}

void log_printf_fl(const char *func_or_file, int line, const char *fmt, ...) {
    unsigned cnt;
    char     buf[MAX_LOG_ENTRY_LEN];
    va_list  ap;


    // Prefix message with func name and line number
    cnt = snprintf(buf, MAX_LOG_ENTRY_LEN, "%s:%d ", func_or_file, line);

    if (cnt > MAX_LOG_ENTRY_LEN) { cnt = MAX_LOG_ENTRY_LEN; }
    _copy_to_logfile(buf, cnt);  // do not include terminating NUL
    SEGGER_RTT_Write(0, buf, cnt);


    // Print message
    if (!fmt) { fmt = ""; }   // if fmt == NULL,
    va_start(ap, fmt);
    cnt = vsnprintf(buf, MAX_LOG_ENTRY_LEN, fmt, ap);
    va_end(ap);

    if (cnt > MAX_LOG_ENTRY_LEN) { cnt = MAX_LOG_ENTRY_LEN; }
    _copy_to_logfile(buf, cnt+1);  // include terminating NUL
    SEGGER_RTT_Write(0, buf, cnt);
    SEGGER_RTT_Write(0, "\n", 1);
}


void log_flush2NV(void) {
    // Implementation specific
}

/*********************************************************************
*
* From Seggar J-Link / J-Trace User Guide
*
* Defines for Cortex-M debug unit
*/
#define ITM_STIM_U32 (*(volatile unsigned int*)0xE0000000) // STIM word access
#define ITM_STIM_U8 (*(volatile char*)0xE0000000) // STIM Byte access
#define ITM_ENA (*(volatile unsigned int*)0xE0000E00) // ITM Enable Register
#define ITM_TCR (*(volatile unsigned int*)0xE0000E80) // ITM Trace Control
// Register
/*********************************************************************
*
* SWO_PrintChar()
*
* Function description
* Checks if SWO is set up. If it is not, return,
* to avoid program hangs if no debugger is connected.
* If it is set up, print a character to the ITM_STIM register
* in order to provide data for SWO.
* Parameters
* c: The character to be printed.
* Notes
* Additional checks for device specific registers can be added.
*/
void SWO_PrintChar(char c) {
    //
    // Check if ITM_TCR.ITMENA is set
    //
    if ((ITM_TCR & 1) == 0) {
    return;
    }
    //
    // Check if stimulus port is enabled
    //
    if ((ITM_ENA & 1) == 0) {
    return;
    }
    //
    // Wait until STIMx is ready,
    // then send data
    //
    //while ((ITM_STIM_U8 & 1) == 0);
    ITM_STIM_U8 = c;
}

/*********************************************************************
*
* SWO_PrintString()
*
* Function description
* Print a string via SWO.
*
*/
void SWO_PrintString(const char *s) {
    //
    // Print out character per character
    //
    while (*s) {
    SWO_PrintChar(*s++);
    }
}


void log_swo_string(const char *s) {
    SWO_PrintString(s);
}

void log_swo_printf(const char *fmt, ...) {
    unsigned cnt;
    char     buf[MAX_LOG_ENTRY_LEN];
    va_list  ap;

    va_start(ap, fmt);
    cnt = vsnprintf(buf, MAX_LOG_ENTRY_LEN, fmt, ap);
    va_end(ap);

    log_swo_string(buf);
}

void log_swo_32(const uint32_t u) {
    //
    // Check if ITM_TCR.ITMENA is set
    //
    if ((ITM_TCR & 1) == 0) {
    return;
    }
    //
    // Check if stimulus port is enabled
    //
    if ((ITM_ENA & 1) == 0) {
    return;
    }
    //
    // Wait until STIMx is ready,
    // then send data
    //
    while ((ITM_STIM_U8 & 1) == 0);
    ITM_STIM_U32 = u;

}









