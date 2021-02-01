/**
 *
 *  Hardware Abstraction Layer for SAMD SERCOM module.
 *
 *  Supports USART, SPI and I2C.
 *
 *  COPYRIGHT NOTICE: (c) 2018 DDPA LLC
 *  All Rights Reserved
 *
 */

#ifndef _samd_serial_H_
#define _samd_serial_H_


/*
 * Common to all serial modes
 */
typedef enum { SAMD_SERIAL_SERCOM0=0, SAMD_SERIAL_SERCOM1, SAMD_SERIAL_SERCOM2, SAMD_SERIAL_INSTANCES } samd_serial_sercom_instance_t;
typedef enum { SAMD_SERIAL_USART=0, SAMD_SERIAL_SPI, SAMD_SERIAL_I2C } samd_serial_mode_t;
typedef void (*samd_serial_callback_t) (int bytes_transferred);
struct samd_serial_object_t;  // opaque type


// Error codes
#define SAMD_SERIAL_ERR_NOERR                 0
#define SAMD_SERIAL_ERR_INVALID_INSTANCE     -1
#define SAMD_SERIAL_ERR_INVALID_OBJECT_PTR   -2
#define SAMD_SERIAL_ERR_INVALID_BUFFER_PTR   -3
#define SAMD_SERIAL_ERR_ZERO_BUFFER_LENGTH   -4
#define SAMD_SERIAL_ERR_SERIAL_MODULE_BUSY   -5



// ==== COMMON SERIAL API ====

/*
 * Returns an error code. If write len is zero, buf is assumed to hold a NUL terminated string.
 */

int samd_serial_read(struct samd_serial_object_t *ser_obj, uint8_t addr, char *buf, size_t len, samd_serial_callback_t cb);
int samd_serial_write(struct samd_serial_object_t *ser_obj, uint8_t addr, char *buf, size_t len, samd_serial_callback_t cb);



// ==== USART API ====

int samd_serial_usartInit(struct samd_serial_object_t **ser_obj_ptr, samd_serial_sercom_instance_t sercom_instance, int baud_rate);


// ==== SPI API ====

int samd_serial_spiInit(struct samd_serial_object_t **ser_obj_ptr, samd_serial_sercom_instance_t sercom_instance, int spi_clk_freq);


/*
 * ==== I2C API ====
 *
 * Call samd_serial_i2cWriteSR instead of the common function samd_serial_write
 * to issie a repeated start instead of a stop after the write operation.
 *
 */

typedef enum { SAMD_SERIAL_I2C_SM, SAMD_SERIAL_I2C_FM, SAMD_SERIAL_I2C_FMP, SAMD_SERIAL_I2C_HSM } samd_serial_i2c_transfer_speed_t;

int samd_serial_i2cMasterInit(struct samd_serial_object_t **ser_obj_ptr, samd_serial_sercom_instance_t sercom_instance, samd_serial_i2c_transfer_speed_t speed_mode);
int samd_serial_i2cWriteSR(struct samd_serial_object_t *ser_obj, uint8_t addr, char *buf, size_t len, samd_serial_callback_t cb);


#endif  /* _samd_serial_H_ */


