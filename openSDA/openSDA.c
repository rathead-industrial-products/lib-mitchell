/*******************************************************************************

    openSDA.c - Support for debugging using openSDA.

    Various FRDM boards are supported, identified by the MCU type found in
    the derivative file.

    MCU_MKL25Z4     FRDM-KL25Z
    MCU_MKL43Z4     FRDM-KL43Z
    MCU_MK22F25612  FRDM-K22F

    COPYRIGHT NOTICE: (c) 2016 DDPA LLC
    All Rights Reserved

 ******************************************************************************/

#include  "derivative.h"
#include  "memory.h"
#include  "openSDA.h"


#if (!defined MCU_MKL43Z4) && (!defined MCU_MK22F25612) && (!defined MCU_MKL25Z4) && (!defined MCU_MKL03Z4)
#error ("Processor not defined for use by Open SDA serial debug")
#endif


NEW_FIFO(tx_buf, 32);
NEW_FIFO(rx_buf, 32);


/***** Private functions *****/
static void _openSDAUARTInit(void);
static bool _openSDAUARTPrime(char c);



/// Initialize the openSDA serial port
void  openSDAInit(void) {
    _openSDAUARTInit();
}


/// Get / Put / Write
void  openSDAWrite(char *s) {

    while (*s) {
        while (!openSDAPut(*s));
        ++s;
    }
}

bool  openSDAPut(char c) {
    bool rslt= true;

    LOCK;
    if (!_openSDAUARTPrime(c)) {
        rslt = fifoPush((TFifo) &tx_buf, c);
    }
    END_LOCK;
    return (rslt);
}

bool  openSDAGet(char *c) {
    return (fifoPop((TFifo) &rx_buf, c));
}


#ifdef MCU_MKL03Z4

/*
 * TODO: Write Drivers
 */

/// Configure UART0 to communicate over openSDA at 125000 baud.
/// The baud rate clock source is the 2 MHz LIRC through MCGIRCLK.
/// TX: Port A2, Pin 28
/// RX: Port A1, Pin 27
static void  _openSDAUARTInit(void) {

}

/// Transmit character if LPUART idle.
static bool _openSDAUARTPrime(char c) {

}

/// LPUART0 Interrupt Handler
void LPUART0_IRQHandler(void) {

}
#endif  /* MCU_MKL03Z4 */


#ifdef MCU_MKL25Z4

/*
 * TODO: Write Drivers
 */

/// Configure UART0 to communicate over openSDA at 125000 baud.
/// The baud rate clock source is the 2 MHz LIRC through MCGIRCLK.
/// TX: Port A2, Pin 28
/// RX: Port A1, Pin 27
static void  _openSDAUARTInit(void) {

}

/// Transmit character if LPUART idle.
static bool _openSDAUARTPrime(char c) {

}

/// LPUART0 Interrupt Handler
void LPUART0_IRQHandler(void) {

}
#endif  /* MCU_MKL25Z4 */


#ifdef MCU_MKL43Z4
/// Configure LPUART0 to communicate over openSDA at 125000 baud.
/// The baud rate clock source is the 2 MHz LIRC through MCGIRCLK.
/// TX: Port A1, Pin 24
/// RX: Port A2, Pin 23
static void  _openSDAUARTInit(void) {

    /*
     * Cannot get to 115200 baud due to low freq (2 MHz) clock source.
     * baudclk = 2 MHz / 16 / 1 = 125000.
     */

    SIM_SOPT2 |= SIM_SOPT2_LPUART0SRC(3); // input clk is MCGIRCLK
    SIM_SCGC5 |= SIM_SCGC5_LPUART0_MASK;  // enable module clock
    LPUART0_BAUD  = 0x00000001;           // SBR = 1
    LPUART0_CTRL  = 0x00ac0000;           // TE, RE, TIE, RIE

    PORTA_PCR1 = 0x00000204;              // Enable RX on pin 23
    PORTA_PCR2 = 0x00000204;              // Enable TX on pin 24
    NVIC_EnableIRQ(LPUART0_IRQn);         // enable interrupts
}

/// Transmit character if LPUART idle.
static bool _openSDAUARTPrime(char c) {
    if (LPUART0_STAT & LPUART_STAT_TC_MASK) {
        LPUART0_DATA = (uint32_t) c;
        LPUART0_CTRL |= LPUART_CTRL_TIE_MASK;
        return (true);
    }
    return (false);
}

/// LPUART0 Interrupt Handler
void LPUART0_IRQHandler(void) {
    char c;

    if (LPUART0_STAT & LPUART_STAT_TDRE_MASK) {
        if (fifoPop((TFifo) &tx_buf, &c) ){
            LPUART0_DATA = (uint32_t) c;
        }
        else {
            LPUART0_CTRL &= ~LPUART_CTRL_TIE_MASK;
        }
    }

    if (LPUART0_STAT & LPUART_STAT_RDRF_MASK) {
        c = (char) (LPUART0_DATA & 0x000000ff);
        fifoPush((TFifo) &rx_buf, c);
    }
}
#endif  /* MCU_MKL43Z4 */


#ifdef MCU_MK22F25612
/// Configure UART1 to communicate over openSDA at 115200 baud.
/// The baud rate clock source is ???.
/// TX: Port E0, Pin 1
/// RX: Port E1, Pin 2
static void  _openSDAUARTInit(void) {

    UART1_BDH = 0x00;
    UART1_BDL = 0x41;                   // SBR  = 65 = 01000001
    UART1_C1  = 0x00;
    UART1_C2  = 0xac;                   // TE, RE, TIE, RIE
    UART1_C3  = 0x00;
    UART1_C4  = 0x03;                   // BRFA = 00011    (3/32)

    PORTE_PCR0 = 0x00000304;            // Enable TX on pin 1
    PORTE_PCR1 = 0x00000304;            // Enable RX on pin 2
    NVIC_EnableIRQ(UART1_RX_TX_IRQn);   // enable interrupts
}

/// Transmit character if UART idle.
static bool _openSDAUARTPrime(char c) {
    if (UART1_S1 & UART_S1_TC_MASK) {
        UART1_D = c;
        UART1_C2 |= UART_C2_TIE_MASK;
        return (true);
    }
    return (false);
}

/// UART1 Interrupt Handler
void UART1_RX_TX_IRQHandler(void) {
    char c;

    if (UART1_S1 & UART_S1_TDRE_MASK) {
        if (fifoPop((TFifo) &tx_buf, &c) ){
            UART1_D = c;
        }
        else {
            UART1_C2 &= ~UART_C2_TIE_MASK;
        }
    }

    if (UART1_S1 & UART_S1_RDRF_MASK) {
        c = UART1_D;
        fifoPush((TFifo) &rx_buf, c);
    }
}
#endif  /* MCU_MK22F25612 */







