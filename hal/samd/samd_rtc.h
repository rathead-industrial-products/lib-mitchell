/**
 *
 *  samd_rtc.h
 *  Hardware Abstraction Layer for Microchip SAMD MCU Real-Time Clock Module.
 *
 *  COPYRIGHT NOTICE: (c) 2019 DDPA LLC
 *  All Rights Reserved
 *
 */


#ifndef _samd_rtc_H_
#define _samd_rtc_H_

#include  <stdbool.h>
#include  <stdint.h>
#include  "sam.h"

typedef void(*voidFuncPtr)(void);

/* Sleep control, requires RTC be configured to generate an interrupt to wake up. Returns milliseconds spent asleep. */
uint32_t  samd_RTCSleep(void);                  // lowest power deep sleep mode.
uint32_t  samd_RTCSleepFor(uint32_t ms);        // configures alarm using ..InterruptAt() to interrupt ms milliseconds
uint32_t  samd_RTCPlayPossum(void);             // spin loop, use in place of samd_RTCSleep for debugging
uint32_t  samd_RTCPlayPossumFor(uint32_t ms);   // spin loop, use in place of samd_RTCSleepFor for debugging


/* Clock Functions - RTC counts seconds in a 22.10 fractional format, delays and interrupts are converted to/from milliseconds */
void      samd_RTCInit(void);
uint32_t  samd_RTCGetClock(void);                   // raw count 22.10 format
void      samd_RTCSetClock(const uint32_t count);   // raw count 22.10 format
void      samd_RTCDelay(const uint32_t ms);
void      samd_RTCInterruptEvery(const uint32_t ms, const voidFuncPtr callback);
void      samd_RTCInterruptAt(const uint32_t ms, const voidFuncPtr callback);
void      samd_RTCInterruptDisable(void);


#endif  /* _samd_rtc_H_ */


