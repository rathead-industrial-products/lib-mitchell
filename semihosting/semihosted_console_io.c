/******************************************************************************

    (c) Copyright 2016 DDPA LLC
    ALL RIGHTS RESERVED.

    semihosted_console_io.c - Redirect console I/O through ARM semihosting interface

    Derived from Freescale semihosted_console_io.c

    For the interface definitions see ARM DUI0471L

 *****************************************************************************/

#include <stdint.h>
#include <string.h>
#include "contract.h"
#include "semihosting.h"


#define PE_CONSOLE_HANDLE       1     // file handle

#define DHCSR                   (* (uint32_t *) 0xE000EDF0)    // not defined anywhere else
#define DHCSR_C_DEBUGEN_MASK    0x00000001

static bool  f_console_open    =  FALSE;
static int   pe_console_handle = 0;


/*
 *  attached - return TRUE if a debugger is attached.
 */
bool  _attached(void) {
    /* DHCSR does not work for M0 */
    //return (DHCSR & DHCSR_C_DEBUGEN_MASK);
    return TRUE;
}


/* shOpen - open a file on the host */
int shOpen(semihost_open_parms * const op) {
    int handle;

    if (_attached()) {
        __asm ("mov   r1, %0"   : : "r"   (op));
        __asm ("mov   r0, %0"   : : "i"   (SH_SYS_OPEN));
        __asm ("bkpt  %0"       : : "i"   (SH_BKPT));
        __asm ("mov   %0, r0"   : "=r" (handle) : );
    }
    return (handle);
}


/* shRead - the contents of a specified file at the current file position */
int shRead(semihost_readwrite_parms * const parms) {
    int r = parms->count;
    if (_attached()) {
        __asm ("mov   r1, %0"   : : "r" (parms));
        __asm ("mov   r0, %0"   : : "i" (SH_SYS_READ));
        __asm ("bkpt  %0"       : : "i" (SH_BKPT));
        __asm ("mov   %0, r0"   : "=r"  (r) : );
    }
    return (r);
}


/* shWrite - the contents of a buffer to a specified file at the current file position */
void shWrite(semihost_readwrite_parms * const parms) {
    if (_attached()) {
        __asm ("mov   r1, %0"   : : "r" (parms));
        __asm ("mov   r0, %0"   : : "i" (SH_SYS_WRITE));
        __asm ("bkpt  %0"       : : "i" (SH_BKPT));
    }
}


/* shIsTTY - verify file handle points to an interactive console */
int shIsTTY(int const handle) {
    int rslt;

    if (_attached()) {
        __asm ("mov   r1,%0"    : : "r" (handle));
        __asm ("mov   r0,%0"    : : "i" (SH_SYS_ISTTY));
        __asm ("bkpt  %0"       : : "i" (SH_BKPT));
        __asm ("mov   %0, r0"   : "=r" (rslt) : );
    }
    return (rslt);
}


/* semihost initialization - open the console and return its handle */
int shOpenConsole(void) {
    semihost_open_parms console;
    int                 hdl_w, hdl_r;

    f_console_open = _attached();

    console.name          = ":tt";
    console.semihost_mode = 4;
    console.name_len      = 3;
    hdl_w = shOpen(&console);    // stdout

    f_console_open &= (shIsTTY(hdl_w) == 1);

    console.semihost_mode = 0;
    hdl_r = shOpen(&console);    // stdin, should be same handle as stdout

    f_console_open &= (hdl_r == hdl_w) & (shIsTTY(hdl_r) == 1);

    return (f_console_open ? hdl_r : 0);
}


/* shWriteC -	write a character to the console */
void shWriteC(const char * const c) {
    if (f_console_open) {
        __asm ("mov		r1, %0"   : : "r" (c));
        __asm ("mov		r0, %0"   : : "i" (SH_SYS_WRITEC));
        __asm ("bkpt  %0"       : : "i" (SH_BKPT));
    }
}


/* shWrite0 -	write a nul-terminated string to the console */
void shWrite0(const char * const str) {
    if (f_console_open) {
        __asm ("mov		r1, %0"   : : "r" (str));
        __asm ("mov		r0, %0"   : : "i" (SH_SYS_WRITE0));
        __asm ("bkpt  %0"       : : "i" (SH_BKPT));
    }
}


/* shClock -  returns number of centiseconds since system started */
int shClock(void) {
    int cs;

    if (_attached()) {
        __asm ("mov   r1, #0");
        __asm ("mov   r0, %0"   : : "i" (SH_SYS_CLOCK));
        __asm ("bkpt  %0"       : : "i" (SH_BKPT));
        __asm ("mov   %0, r0"   : "=r"  (cs) : );
    }
    else {
        cs = -1;
    }
    return (cs);
}


/* shReadC -  read a character from the console */
char shReadC(void) {
    char c;

    if (f_console_open) {
        __asm ("mov   r1, #0");
        __asm ("mov   r0, %0"   : : "i" (SH_SYS_READC));
        __asm ("bkpt  %0"       : : "i" (SH_BKPT));
        __asm ("mov   %0, r0"   : "=r"  (c) : );
    }
    else {
        c = '\0';
    }
    return (c);
}



/*
 *	shReadConsole	-	read from the console into 'buffer' until end-of-line
 *	                or 'count' characters have been read. Buffer must be
 *	                large enough to hold 'count' characters plus a terminating
 *	                NUL.
 *
 *	                Return 0 if the debug console is not attached.
 *
 */
uint8_t shReadConsole(char * const buffer, uint8_t count){
    char    c;
    uint8_t num_chars = 0;

    if (!f_console_open) {
        buffer[0] = '\0';
        return (0);
    }

    while (count) {
        c = shReadC();
        if ((c == '\n') || (c == '\r')) {
            break;
        }
        if (c != '\0') {
            buffer[num_chars] = c;
            ++num_chars;
            --count;
        }
    }
    buffer[num_chars] = '\0';
    return (num_chars);
}

/*
 * P & E Console operations overlayed on standard file I/O
 */

int shPEOpenConsole(void) {
    pe_console_handle = shOpenConsole();
    return (pe_console_handle);
}

void shPEWrite0(const char * const str) {
    semihost_readwrite_parms console;;

    console.handle  = pe_console_handle;
    console.data    = (char *) str;
    console.count   = strlen(str);

    if (f_console_open) {
        shWrite(&console);
    }
}


#ifdef UNIT_TEST
const char WRITEC_STR[] = "shWriteC test\r";

int sh_UNIT_TEST(void) {
  bool f_passed = TRUE;

  f_passed &= (shPEOpenConsole() != 0);

  shPEWrite0("shWrite0 test");

  return (f_passed ? 0 : 1);
}

#endif /* UNIT_TEST */


