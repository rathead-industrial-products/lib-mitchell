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

#include  <stdbool.h>
#include  <stdint.h>
#include  <stddef.h>
#include  <string.h>
#include  <sam.h>
#include  "samd_serial.h"
#include  "samd_port.h"
//#include  <hpl_gclk_base.h>
//#include  <hpl_pm_base.h>
#include  "../setup/include/utils.h"

typedef enum { SAMD_SERIAL_IDLE=0, SAMD_SERIAL_I2C_READ, SAMD_SERIAL_I2C_WRITE } samd_serial_activity_t;

typedef struct {
    uint8_t                 addr_rw;
    char                   *buf;
    uint8_t                 len;
    uint8_t                 idx;
    bool                    i2c_sr;
    samd_serial_callback_t  cb;
    samd_serial_activity_t  busy;
} samd_serial_tranfer_t;

typedef struct samd_serial_object_t {
    Sercom *                        instance;
    samd_serial_mode_t              mode;
    bool                            i2c_hs;
    volatile samd_serial_tranfer_t  xfer;
} samd_serial_object_t;

static Sercom * const         sercom_instance_list[SAMD_SERIAL_INSTANCES]    = { SERCOM0, SERCOM1, SERCOM2 };
static const IRQn_Type        interrupt_number_list[SAMD_SERIAL_INSTANCES]   = { SERCOM0_IRQn, SERCOM1_IRQn, SERCOM2_IRQn };
static samd_serial_object_t   sercom[SAMD_SERIAL_INSTANCES]                  = { 0 };

#define SAMD_SERIAL_MAX_UNSPECIFIED_WRITE_STR_LEN  64    // max len for a NUL terminated write string with len = 0

/*
 * Forward function declarations
 */
static int _read_usart(samd_serial_object_t *ser_obj);
static int _read_spi(samd_serial_object_t *ser_obj);
static int _read_write_i2c(samd_serial_object_t *ser_obj);
static int _write_usart(samd_serial_object_t *ser_obj);
static int _write_spi(samd_serial_object_t *ser_obj);


/******************************************************************************
 *
 * Read len bytes and write into buf.
 *
 * Return immediately and execute callback when the read is complete or the
 * transfer is aborted.
 *
 * If cb is NULL, do not return until read operation
 * is complete.
 *
 *****************************************************************************/
int samd_serial_read(struct samd_serial_object_t *ser_obj, uint8_t addr, char *buf, size_t len, samd_serial_callback_t cb) {
    if (ser_obj == NULL)      { return (SAMD_SERIAL_ERR_INVALID_OBJECT_PTR); }
    if (buf == NULL)          { return (SAMD_SERIAL_ERR_INVALID_BUFFER_PTR); }
    if (len == 0)             { return (SAMD_SERIAL_ERR_ZERO_BUFFER_LENGTH); }
    if (ser_obj->xfer.busy)   { return (SAMD_SERIAL_ERR_SERIAL_MODULE_BUSY); }
    int err;

    ser_obj->xfer.cb      = cb;
    ser_obj->xfer.addr_rw = (addr << 1) | 1;
    ser_obj->xfer.buf     = buf;
    ser_obj->xfer.len     = len;
    ser_obj->xfer.idx     = 0;
    ser_obj->xfer.busy    = SAMD_SERIAL_I2C_READ;

    switch (ser_obj->mode) {

        case SAMD_SERIAL_USART:
            err = SAMD_SERIAL_ERR_NOERR;
            break;

        case SAMD_SERIAL_SPI:
            err = SAMD_SERIAL_ERR_NOERR;
            break;

        case SAMD_SERIAL_I2C:
            err = _read_write_i2c(ser_obj);
            break;

        default:
            assert(0);
            break;
    }
    return (err);
}


/******************************************************************************
 *
 * Write len bytes from buf, or if len is zero write up to SAMD_SERIAL_MAX_UNSPECIFIED_WRITE_STR_LEN
 * bytes or until a NUL is reached.
 *
 * Return immediately and execute callback when the write is complete or the
 * transfer is aborted.
 *
 * If cb is NULL, do not return until write operation is complete.
 *
 *****************************************************************************/
int samd_serial_write(struct samd_serial_object_t *ser_obj, uint8_t addr, char *buf, size_t len, samd_serial_callback_t cb) {
    if (ser_obj == NULL)      { return (SAMD_SERIAL_ERR_INVALID_OBJECT_PTR); }
    if (buf == NULL)          { return (SAMD_SERIAL_ERR_INVALID_BUFFER_PTR); }
    if (ser_obj->xfer.busy)   { return (SAMD_SERIAL_ERR_SERIAL_MODULE_BUSY); }
    int err;

    if (len == 0) { while (buf[len] && (len < SAMD_SERIAL_MAX_UNSPECIFIED_WRITE_STR_LEN)) { ++len; } }   // strnlen()

    ser_obj->xfer.cb      = cb;
    ser_obj->xfer.addr_rw = (addr << 1) | 0;
    ser_obj->xfer.buf     = buf;
    ser_obj->xfer.len     = len;
    ser_obj->xfer.idx     = 0;
    // ser_obj->xfer.sr cleared by interrupt handler and set by samd_serial_i2cWriteSR
    ser_obj->xfer.busy = SAMD_SERIAL_I2C_WRITE;

    switch (ser_obj->mode) {

        case SAMD_SERIAL_USART:
            err = SAMD_SERIAL_ERR_NOERR;
            break;

        case SAMD_SERIAL_SPI:
            err = SAMD_SERIAL_ERR_NOERR;
            break;

        case SAMD_SERIAL_I2C:
            err = _read_write_i2c(ser_obj);
            break;

        default:
            assert(0);
            break;
    }
    return (err);
}


/******************************************************************************
 ******************************************************************************
 *
 *      USART SPECIFIC
 *
 ******************************************************************************
 *****************************************************************************/
int samd_serial_usartInit(struct samd_serial_object_t **ser_obj_ptr, samd_serial_sercom_instance_t sercom_instance, int baud_rate) {
}

static void _usartHandler(samd_serial_object_t *ser_obj) {
    assert(0);
}


/******************************************************************************
 ******************************************************************************
 *
 *      SPI SPECIFIC
 *
 ******************************************************************************
 *****************************************************************************/
int samd_serial_spiInit(struct samd_serial_object_t **ser_obj_ptr, samd_serial_sercom_instance_t sercom_instance, int spi_clk_freq) {
}

static void _spiHandler(samd_serial_object_t *ser_obj) {
    assert(0);
}


/******************************************************************************
 ******************************************************************************
 *
 *      I2C SPECIFIC
 *
 *  The I2C interface supports four standard baud rates:
 *  MODE            fSCL      Period  SDA hold  tRISE max
 *  Standard Mode   100 KHz   10  us  600 ns    350 ns per datasheet
 *  Fast Mode       400 KHz   2.5 us  600 ns    350 ns
 *  Fast Mode Plus  1 MHz     1.0 us   75 ns    100 ns
 *  High Speed Mode 3.4 MHz   294 ns   75 ns    60 ns
 *
 *  Measured rise time with 2.2K resistors on an Adafruit tripler breakout 100 ns.
 *
 *  100 KHz @ 8 MHz GCLK, SCL Hi to Lo ratio 1:1
 *  ---------------------------------------------
 *  tLO + tHI + tRISE = 1/100K = 10 us
 *  BAUD + BAUDLOW = (fGCLK * (1/FSCL – tRISE)) – 10
 *  BAUD + BAUDLOW = (8 * (10 – 0.35)) – 10
 *  BAUD + BAUDLOW = 67
 *  BAUD    = 34
 *  BAUDLOW = 34
 *
 *  400 KHz @ 48 MHz GCLK, SCL Hi to Lo ratio 1:1
 *  ---------------------------------------------
 *  tLO + tHI + tRISE = 1/100K = 2.5 us
 *  BAUD + BAUDLOW = (fGCLK * (1/FSCL – tRISE)) – 10
 *  BAUD + BAUDLOW = (48 * (2.5 – 0.35)) – 10
 *  BAUD + BAUDLOW = 93
 *  BAUD    = 47
 *  BAUDLOW = 47
 *
 *  1 MHz @ 48 MHz GCLK, SCL Hi to Lo ratio 1:2
 *  ---------------------------------------------
 *  tLO + tHI + tRISE = 1/1M = 1 us
 *  BAUD + BAUDLOW = (fGCLK * (1/FSCL – tRISE)) – 10
 *  BAUD + BAUDLOW = (48 * (1.0 – 0.1)) – 10
 *  BAUD + BAUDLOW = 33
 *  BAUD    = 11
 *  BAUDLOW = 22
 *
 *
 *  3.4 MHz @ 48 MHz GCLK, SCL Hi to Lo ratio 1:2
 *  ---------------------------------------------
 *  BAUD + BAUDLOW = (fGCLK/FSCL) – 2
 *  BAUD + BAUDLOW = (48/3.4) - 2
 *  BAUD + BAUDLOW = 12
 *  BAUD    = 4
 *  BAUDLOW = 8
 *
 ******************************************************************************
 *****************************************************************************/

#define SAMD_SERIAL_I2C_BAUD_SM       34
#define SAMD_SERIAL_I2C_BAUDLOW_SM    34
#define SAMD_SERIAL_I2C_BAUD_FM       47
#define SAMD_SERIAL_I2C_BAUDLOW_FM    47
#define SAMD_SERIAL_I2C_BAUD_FMP      11
#define SAMD_SERIAL_I2C_BAUDLOW_FMP   22
#define SAMD_SERIAL_I2C_BAUD_HSM      4
#define SAMD_SERIAL_I2C_BAUDLOW_HSM   8

#define SAMD_SERIAL_I2C_CMD_SR  0x01  // issue repeated start
#define SAMD_SERIAL_I2C_CMD_RD  0x02  // perform a read byte operation
#define SAMD_SERIAL_I2C_CMD_P   0x03  // issue stop condition

#define SAMD_SERIAL_I2C_SDA_HOLD_75NS   0x01  //  50-100ns hold time
#define SAMD_SERIAL_I2C_SDA_HOLD_450NS  0x02  // 300-600ns hold time
#define SAMD_SERIAL_I2C_SDA_HOLD_600NS  0x03  // 400-800ns hold time

#define SAMD_SERIAL_I2C_HS_MASTER_CODE  0x08

#define SAMD_SERIAL_I2C_SDA             SAMD_PORT_PIN(SAMD_PORTA, 22)
#define SAMD_SERIAL_I2C_SCL             SAMD_PORT_PIN(SAMD_PORTA, 23)

#define I2C_SYNC while(ser_obj->instance->I2CM.SYNCBUSY.reg) // wait for all sync bits to clear


/******************************************************************************
 *
 * Initialize a SERCOM instance as an I2C master running at one of
 * four standard speeds.
 *
 *****************************************************************************/
int samd_serial_i2cMasterInit(struct samd_serial_object_t **ser_obj_ptr, samd_serial_sercom_instance_t sercom_instance, samd_serial_i2c_transfer_speed_t speed_mode) {
    samd_serial_object_t * ser_obj = &sercom[sercom_instance];

    if (sercom_instance >= SAMD_SERIAL_INSTANCES) { return (SAMD_SERIAL_ERR_INVALID_INSTANCE);   }
    if (ser_obj == NULL)                          { return (SAMD_SERIAL_ERR_INVALID_OBJECT_PTR); }

    *ser_obj_ptr = ser_obj;
    ser_obj->instance = sercom_instance_list[sercom_instance];
    ser_obj->mode     = SAMD_SERIAL_I2C;

    switch (sercom_instance) {
        case SAMD_SERIAL_SERCOM0:
            break;

        case SAMD_SERIAL_SERCOM1:
            break;

        case SAMD_SERIAL_SERCOM2:
        samd_port_AltFunc(SAMD_SERIAL_I2C_SDA, SAMD_PORT_PERIPHERAL_FUNC_D);
        samd_port_AltFunc(SAMD_SERIAL_I2C_SCL, SAMD_PORT_PERIPHERAL_FUNC_D);
        // configure clocks - slow clock unused
        // 48 MHz except for standard mode which is 8 MHz due to limited BAUD counter range
        PM->APBCMASK.reg |= PM_APBCMASK_SERCOM2;
        GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( SERCOM2_GCLK_ID_CORE ) |
                            ((speed_mode != SAMD_SERIAL_I2C_SM) ? GCLK_CLKCTRL_GEN_GCLK0 : GCLK_CLKCTRL_GEN_GCLK3) |
                            GCLK_CLKCTRL_CLKEN ;
        break;

        default:
            assert(0);
            break;
    }


    /*
     * Cannot get reads to work with SCLSM mode set. Not an issue with lower speeds, but is required for high-speed mode
     */
    switch (speed_mode) {
        case SAMD_SERIAL_I2C_SM:
            // Insufficient range in BAUD counters for 48 MHz clock, 8 MHz was set above
            ser_obj->instance->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_SPEED (0x00) | SERCOM_I2CM_CTRLA_SDAHOLD(SAMD_SERIAL_I2C_SDA_HOLD_600NS) /* | SERCOM_I2CM_CTRLA_SCLSM */ | SERCOM_I2CM_CTRLA_MODE_I2C_MASTER;
            ser_obj->instance->I2CM.BAUD.reg  = SERCOM_I2CM_BAUD_BAUD(SAMD_SERIAL_I2C_BAUD_SM) | SERCOM_I2CM_BAUD_BAUDLOW(SAMD_SERIAL_I2C_BAUDLOW_SM);
            break;

        case SAMD_SERIAL_I2C_FM:
            ser_obj->instance->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_SPEED (0x00) | SERCOM_I2CM_CTRLA_SDAHOLD(SAMD_SERIAL_I2C_SDA_HOLD_600NS) /* | SERCOM_I2CM_CTRLA_SCLSM */ | SERCOM_I2CM_CTRLA_MODE_I2C_MASTER;
            ser_obj->instance->I2CM.BAUD.reg  = SERCOM_I2CM_BAUD_BAUD(SAMD_SERIAL_I2C_BAUD_FM) | SERCOM_I2CM_BAUD_BAUDLOW(SAMD_SERIAL_I2C_BAUDLOW_FM);
            break;

        case SAMD_SERIAL_I2C_FMP:
            ser_obj->instance->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_SPEED (0x01) | SERCOM_I2CM_CTRLA_SDAHOLD(SAMD_SERIAL_I2C_SDA_HOLD_75NS) /* | SERCOM_I2CM_CTRLA_SCLSM */ | SERCOM_I2CM_CTRLA_MODE_I2C_MASTER;
            ser_obj->instance->I2CM.BAUD.reg  = SERCOM_I2CM_BAUD_BAUD(SAMD_SERIAL_I2C_BAUD_FMP) | SERCOM_I2CM_BAUD_BAUDLOW(SAMD_SERIAL_I2C_BAUDLOW_FMP);
           break;

        case SAMD_SERIAL_I2C_HSM:
            ser_obj->i2c_hs = true;
            ser_obj->instance->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_SPEED (0x02) | SERCOM_I2CM_CTRLA_SDAHOLD(SAMD_SERIAL_I2C_SDA_HOLD_75NS) | SERCOM_I2CM_CTRLA_SCLSM | SERCOM_I2CM_CTRLA_MODE_I2C_MASTER;
            ser_obj->instance->I2CM.BAUD.reg  = SERCOM_I2CM_BAUD_BAUD(SAMD_SERIAL_I2C_BAUD_FM)    | SERCOM_I2CM_BAUD_BAUDLOW(SAMD_SERIAL_I2C_BAUD_FM) |
                                                SERCOM_I2CM_BAUD_HSBAUD(SAMD_SERIAL_I2C_BAUD_HSM) | SERCOM_I2CM_BAUD_HSBAUDLOW(SAMD_SERIAL_I2C_BAUDLOW_HSM);
            break;
    }

    /* smart mode enabled by setting the bit SMEN as 1 */
    ser_obj->instance->I2CM.CTRLB.reg = SERCOM_I2CM_CTRLB_SMEN; I2C_SYNC;

    /* SERCOM2 peripheral enabled by setting the ENABLE bit as 1*/
    ser_obj->instance->I2CM.CTRLA.reg |= SERCOM_I2CM_CTRLA_ENABLE; I2C_SYNC;

    /* bus state is forced into idle state */
    ser_obj->instance->I2CM.STATUS.bit.BUSSTATE = 0x1; I2C_SYNC;

    /* Both master on bus and slave on bus interrupt is enabled */
    ser_obj->instance->I2CM.INTENSET.reg = SERCOM_I2CM_INTENSET_MB | SERCOM_I2CM_INTENSET_SB;

    //system_interrupt_enable(SERCOM2_IRQn);
    NVIC_EnableIRQ(interrupt_number_list[sercom_instance]);

    return (0);
}

/******************************************************************************
 *
 * Write chars from buf.
 *
 * Same as samd_serial_write() but terminates write with a repeated start
 * instead of a stop.
 *
 *****************************************************************************/
int samd_serial_i2cWriteSR(struct samd_serial_object_t *ser_obj, uint8_t addr, char *buf, size_t len, samd_serial_callback_t cb) {
    if (ser_obj) { ser_obj->xfer.i2c_sr = true; }
    return (samd_serial_write(ser_obj, addr, buf, len, cb));
}


/******************************************************************************
 *
 * Protocol specific read and write routines
 *
 *****************************************************************************/
static int _read_write_i2c(samd_serial_object_t *ser_obj) {
    if (ser_obj->xfer.idx < (ser_obj->xfer.len - 1)) { }

    // NACK read if only one byte, otherwise ACK and interrupt handler will NACK on last byte
    if (ser_obj->xfer.len == 1 ) { ser_obj->instance->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT; }
    else                         { ser_obj->instance->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT; }

    // enter high-speed mode or start read operation, data transfer done in interrupt handler
    if (ser_obj->i2c_hs) { ser_obj->instance->I2CM.ADDR.reg = SAMD_SERIAL_I2C_HS_MASTER_CODE; }
    else                 { ser_obj->instance->I2CM.ADDR.reg = ser_obj->xfer.addr_rw; }

    if (ser_obj->xfer.cb == NULL) {   // no callback, spin until tranfer complete
        while (ser_obj->xfer.busy);
        return (ser_obj->xfer.idx);
    }
    return (SAMD_SERIAL_ERR_NOERR);
}



/******************************************************************************
 *
 * Interrupt handler.
 *
 *****************************************************************************/
static void _i2cHandler(samd_serial_object_t *ser_obj) {

    // error
    if (ser_obj->instance->I2CM.INTFLAG.bit.ERROR) {
        ser_obj->instance->I2CM.CTRLB.bit.CMD = SAMD_SERIAL_I2C_CMD_P;    // drive stop on bus
        ser_obj->xfer.busy = SAMD_SERIAL_IDLE;                            // module is now idle
        if (ser_obj->xfer.cb) { ser_obj->xfer.cb(ser_obj->xfer.idx); }    // execute callback
        ser_obj->instance->I2CM.INTFLAG.reg = SERCOM_I2CM_INTFLAG_ERROR;  // clear interrupt
    }

    // master on bus
    else if (ser_obj->instance->I2CM.INTFLAG.bit.MB) {

        if (ser_obj->instance->I2CM.STATUS.bit.RXNACK) {
            // hs master code was NACK'd, now in high speed mode, send slave address
            if (ser_obj->i2c_hs && ser_obj->xfer.addr_rw) {
                ser_obj->instance->I2CM.ADDR.reg = ser_obj->xfer.addr_rw | SERCOM_I2CM_ADDR_HS;
                ser_obj->xfer.addr_rw = 0; // flag to treat subsequent NACKs normally
            }
            else {  // normal protocol address or data NACK'd
                ser_obj->instance->I2CM.CTRLB.bit.CMD = SAMD_SERIAL_I2C_CMD_P;  // drive stop on bus
                ser_obj->xfer.busy = SAMD_SERIAL_IDLE;                          // module is now idle
                if (ser_obj->xfer.cb) { ser_obj->xfer.cb(ser_obj->xfer.idx); }  // execute callback
            }
        }

        // no error, write operation in progress, transmit next byte
        else if (ser_obj->xfer.idx < ser_obj->xfer.len) {
            ser_obj->instance->I2CM.DATA.reg = ser_obj->xfer.buf[ser_obj->xfer.idx++]; I2C_SYNC;
        }

        // last byte of write has been sent, issue stop or repeated start, clean up
        else {
            if (ser_obj->xfer.i2c_sr) {
                // do not issue repeated start command, it will transmit the previous address
                // the next write to the addr reg will trigger a repeated start
                ser_obj->instance->I2CM.INTFLAG.reg = SERCOM_I2CM_INTFLAG_MB;  // clear interrupt
            }
            else {
                ser_obj->instance->I2CM.CTRLB.bit.CMD = SAMD_SERIAL_I2C_CMD_P;  I2C_SYNC;
            }
            ser_obj->xfer.i2c_sr = false;  // always stop unless samd_serial_i2cWriteSR() called instead of write()
            ser_obj->xfer.busy = SAMD_SERIAL_IDLE;
            if (ser_obj->xfer.cb) { ser_obj->xfer.cb(ser_obj->xfer.idx); }
        }
    }

    // slave on bus interrupt, read operation in progress
    else if (ser_obj->instance->I2CM.INTFLAG.bit.SB) {
        // last byte received, NACK has already been sent
        if (ser_obj->instance->I2CM.CTRLB.reg & SERCOM_I2CM_CTRLB_ACKACT) {
            ser_obj->instance->I2CM.CTRLB.bit.CMD = SAMD_SERIAL_I2C_CMD_P; I2C_SYNC;              // drive stop on bus
            ser_obj->xfer.buf[ser_obj->xfer.idx++] = ser_obj->instance->I2CM.DATA.reg; I2C_SYNC;  // read data
            ser_obj->xfer.busy = SAMD_SERIAL_IDLE;                                      // idle sercomm
            if (ser_obj->xfer.cb) { ser_obj->xfer.cb(ser_obj->xfer.len); }              // execute callback
        }

        // read still in progress, ACK has already been sent
        else {
            // next to last byte received, NACK the next byte
            if (ser_obj->xfer.idx >= (ser_obj->xfer.len - 1)) {
                ser_obj->instance->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT; I2C_SYNC;
            }
            ser_obj->xfer.buf[ser_obj->xfer.idx++] = ser_obj->instance->I2CM.DATA.reg; I2C_SYNC;  // read data
        }
        // not needed?? ser_obj->instance->I2CM.CTRLB.bit.CMD = SAMD_SERIAL_I2C_CMD_RD; I2C_SYNC;
    }

    else {
        assert (0);
    }
}


/******************************************************************************
 ******************************************************************************
 *
 *      COMMON INTERRUPT HANDLER ENTRY AND DISPATCH
 *
 ******************************************************************************
 *****************************************************************************/
static void _serialHandlerDispatch(unsigned instance) {
    switch (sercom[instance].mode) {
        case SAMD_SERIAL_USART:
            _usartHandler(&sercom[instance]);
            break;
        case SAMD_SERIAL_SPI:
            _spiHandler(&sercom[instance]);
            break;
        case SAMD_SERIAL_I2C:
            _i2cHandler(&sercom[instance]);
            break;
    }
}

void SERCOM0_Handler(void) {
    _serialHandlerDispatch(0);
}

void SERCOM1_Handler(void) {
    _serialHandlerDispatch(1);
}

void SERCOM2_Handler(void) {
    _serialHandlerDispatch(2);
}


