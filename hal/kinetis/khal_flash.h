/**
 *
 *  @file  khal_flash.h
 *  @brief Hardware Abstraction Layer for Kinetis MCU FLASH module.
 *
 *  COPYRIGHT NOTICE: (c) 2016 DDPA LLC
 *  All Rights Reserved
 *
 */


#ifndef _khal_flash_H_
#define _khal_flash_H_

#include  <stdbool.h>
#include  <stdint.h>
#include  "derivative.h"


// ==== Typedefs ====

/// Flash Configuration Field.
/// 16 byte field located at 0x400 that stores default configuration and security information.
typedef struct khal_flash_configuration_field_t {
    uint8_t backdoor_key[8];
    uint8_t fproto[4];
    uint8_t fsec;
    uint8_t fopt;
    uint8_t rsvd[2];
} khal_flash_configuration_field_t;


// ==== Defines ====



// ==== API ====



#endif  /* _khal_flash_H_ */


