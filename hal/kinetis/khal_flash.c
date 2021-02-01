/**
 *
 *  Hardware Abstraction Layer for Kinetis PORT & GPIO modules.
 *
 *  COPYRIGHT NOTICE: (c) 2016 DDPA LLC
 *  All Rights Reserved
 *
 */


#include  <stdbool.h>
#include  <stdint.h>
#include  "contract.h"
#include  "derivative.h"
#include  "khal_flash.h"


// ==== Flash Configuration Field Initialization ====
const khal_flash_configuration_field_t __attribute__((section (".cfmconfig"))) flash_config = {
        { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },   ///< backdoor key
        { 0xff, 0xff, 0xff, 0xff },                           ///< FPROT0-3 - no flash protection
          0xfe,                                               ///< FSEC - unsecure the device
          0x3d,                                               ///< FOPT - disable the ROM bootloader
        { 0xff, 0xff } };                                     ///< reserved
                                      ///< FSEC - flash security byte



// ==== Defines ====



// ==== API ====







