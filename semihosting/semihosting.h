/******************************************************************************

    (c) Copyright 2016 DDPA LLC
    ALL RIGHTS RESERVED.

    semihosting.h - Interface to ARM debugger I/O.

    Derived from Freescale semihosting.h

    For the interface definitions see ARM DUI0471L

 *****************************************************************************/

#ifndef _semihosting_H_
#define _semihosting_H_

#include "contract.h"

#define SH_BKPT             0xab


#define SH_angel_SWIreason_EnterSVC           0x17
#define SH_angel_SWIreason_ReportException    0x18
#define SH_SYS_CLOSE        0x02
#define SH_SYS_CLOCK        0x10
#define SH_SYS_ELAPSED      0x30
#define SH_SYS_ERRNO        0x13
#define SH_SYS_FLEN         0x0c
#define SH_SYS_GET_CMDLINE  0x15
#define SH_SYS_HEAPINFO     0x16
#define SH_SYS_ISERROR      0x08
#define SH_SYS_ISTTY        0x09
#define SH_SYS_OPEN         0x01
#define SH_SYS_READ         0x06
#define SH_SYS_READC        0x07
#define SH_SYS_REMOVE       0x0e
#define SH_SYS_RENAME       0x0f
#define SH_SYS_SEEK         0x0a
#define SH_SYS_SYSTEM       0x12
#define SH_SYS_TICKFREQ     0x31
#define SH_SYS_TIME         0x11
#define SH_SYS_TMPNAM       0x0d
#define SH_SYS_WRITE        0x05
#define	SH_SYS_WRITEC       0x03
#define	SH_SYS_WRITE0	      0x04

#define SH_CMDLINE_MAX 256


/*
 * Semihosting service call interface structures for functions that need
 * more than simple parameters in r0, r1.
 */

/* SH_SYS_OPEN */
typedef struct {
	char * name;			    // pointer to filename, null terminated
	int semihost_mode;	  // file mode in semihost format
	int name_len;			    // filename length excluding null character
} semihost_open_parms;

/* SH_SYS_READ, SH_SYS_WRITE */
typedef struct {
	int handle;				    // file handle previously opened
  char * data;	        // pointer to memory of data to be written
	int count;				    // count of number of bytes to be written
} semihost_readwrite_parms;

/* SH_SYS_SEEK */
typedef struct  {
	int handle;				    // file handle previously opened
	unsigned long pos;		// absolute file position to seek to
} semihost_seek_parms;

/* SH_SYS_TMPNAM */
typedef struct {
	char *name;				    // storage to host returned tmp name
	int targetid;			    // unique target id for each tmp name
	int len;				      // size of storage, usually L_tmpnam
} semihost_tmpnam_parms;

/* SH_SYS_RENAME */
typedef struct {
	char *oldname;			  // pointer to old file name
	int olen;				      // length of old file name
	char *newname;			  // pointer to new file name
	int nlen;				      // length of new file name
} semihost_rename_parms;

/* SH_SYS_GET_CMDLINE */
typedef struct {
	char *cmdline;			  // storage for cmd line
	int len;				      // max length of buffer (256)

} semihost_cmdline_parms;

/* SH_SYS_REMOVE */
typedef struct {
	char *name;				// storage for cmd line
	int len;				// max length of buffer (256)
} semihost_remove_parms;

/* SH_SYS_SYSTEM */
typedef struct {
	char *cmd;				// system command to execute
	int len;				// length of command
} semihost_system_parms;



int     shOpen(semihost_open_parms * const op);
void    shWrite(semihost_readwrite_parms * const parms);
int     shRead(semihost_readwrite_parms * const parms);


/* Standard console operations do not work with P&E debugger */
int     shOpenConsole(void);
int     shIsTTY(int const handle);
void    shWriteC(const char * const c);
void    shWrite0(const char * const str);
char    shReadC(void);
uint8_t shReadConsole(char * const buffer, uint8_t count);

/* P&E console operations overlayed on standard file I/O */
int     shPEOpenConsole(void);
void    shPEWrite0(const char * const str);




#endif  /* _semihosting_H_ */
