/**
 *
 *  @file  logging.h
 *  @brief printf style logging utility.
 *
 *  Log comments to debug output, volatile memory, and non-volatile memory.
 *
 *  Comments are always sent to the Segger RTT terminal. If the
 *  J-Link probe is not attached, this operation is a nop.
 *
 *  If there is a log area defined in SRAM, comments are logged
 *  to it as a circular buffer.
 *
 *  The contents of the SRAM buffer can be flushed to non-volatile memory.
 *
 *  A separate utility for outputting via the SWO port is provided.
 *
 *
 *  COPYRIGHT NOTICE: (c) 2018 DDPA LLC
 *  All Rights Reserved
 *
 */


#ifndef _logging_H_
#define _logging_H_

#include <stdint.h>


// ==== Logging Functions ====

#define LOG_PRINTF_FUNC_LINE(fmt, ...)  log_printf_fl(__func__, __LINE__, fmt, __VA_ARGS__)

/// Log the printf style message.
void log_printf(const char *fmt, ...);
void log_printf_fl(const char *func_or_file, int line, const char *fmt, ...);

// Flush log buffer to NV memory
void log_flush2NV(void);


// ==== SWO Functions ====

void log_swo_printf(const char *fmt, ...);
void log_swo_string(const char *s);
void log_swo_32(const uint32_t u);

#endif  /* _logging_H_ */


