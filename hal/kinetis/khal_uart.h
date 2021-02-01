/**
 *
 *  @file  khal_uart.h
 *  @brief Hardware Abstraction Layer for Kinetis MCU UART modules.
 *
 *  COPYRIGHT NOTICE: (c) 2016 DDPA LLC
 *  All Rights Reserved
 *
 */


#ifndef _khal_uart_H_
#define _khal_uart_H_

#include  <stdbool.h>
#include  <stdint.h>
#include  "derivative.h"


// ==== Typedefs ====

typedef enum khal_lpuart_br_clk_src_t {
    KHAL_LPUART_BR_CLK_SRC_DIS      = 0,
    KHAL_LPUART_BR_CLK_SRC_IRC48M   = 1,
    KHAL_LPUART_BR_CLK_SRC_OSCERCLK = 2,
    KHAL_LPUART_BR_CLK_SRC_MCGIRCLK = 3,
    KHAL_LPUART_BR_CLK_SRC_ERR
} khal_lpuart_br_clk_src_t;


// ==== DEFINES ====


/* defining KHAL_LPUART_INSTALL_IRQ_HANDLER_LPUARTx instantiates tx and rx fifos and uses the default irq handler */
#define KHAL_LPUART_INSTALL_IRQ_HANDLER_LPUART0
#define KHAL_LPUART_TX_FIFO_SIZE_LPUART0              40
#define KHAL_LPUART_RX_FIFO_SIZE_LPUART0              40

#define KHAL_LPUART_INSTALL_IRQ_HANDLER_LPUART1
#undef  KHAL_LPUART_INSTALL_IRQ_HANDLER_LPUART1
#define KHAL_LPUART_TX_FIFO_SIZE_LPUART1              40
#define KHAL_LPUART_RX_FIFO_SIZE_LPUART1              40



// ==== LPUART API ====

/// Initialize the LPUART.
void khal_lpuart_Init(LPUART_MemMapPtr uart,
                      PORT_MemMapPtr tx_port, uint32_t tx_pin, uint32_t tx_mux,
                      PORT_MemMapPtr rx_port, uint32_t rx_pin, uint32_t rx_mux,
                      khal_lpuart_br_clk_src_t clk_src, uint32_t const sourceClockInHz,
                      uint32_t const desiredBaudRate);

/// Enable (true) or disable (false) the transmitter and receiver.
void khal_lpuart_RxTxEn(LPUART_MemMapPtr uart, bool const enable);

/// Enable (true) or disable (false) the transmitter and receiver interrupts.
void khal_lpuart_IntEn(LPUART_MemMapPtr uart, uint32_t priority, bool const enable);

/// Select the source of the baud rate clock.
void khal_lpuart_SelectBRClk(LPUART_MemMapPtr uart, khal_lpuart_br_clk_src_t clk_src);

/// Set the baud rate.
/// Return true if baud rate was set within 5% of desiredBaudRate, false otherwise.
bool khal_lpuart_SetBaudRate(LPUART_MemMapPtr uart, uint32_t const sourceClockInHz, uint32_t const desiredBaudRate);

/// Return true if the transmitter is idle.
bool khal_lpuart_TxIdle(LPUART_MemMapPtr uart);

/// Return true if the transmitter can accept more data.
bool khal_lpuart_TxBufEmpty(LPUART_MemMapPtr uart);

/// Return true if there is data available from the receiver.
bool khal_lpuart_RxBufFull(LPUART_MemMapPtr uart);

/// Write data to the transmitter.
void khal_lpuart_PutChar(LPUART_MemMapPtr uart, char data);

/// Get data from the receiver.
char khal_lpuart_GetChar(LPUART_MemMapPtr uart);

/// Write a string to the tx fifo (if enabled by #defining KHAL_LPUART_INSTALL_IRQ_HANDLER_LPUARTx).
/// Return false if there is insufficient room int he fifo for the string.
bool khal_lpuart_PutFifo(LPUART_MemMapPtr uart, char *str);

/// Get a character from the rx fifo (if enabled by #defining KHAL_LPUART_INSTALL_IRQ_HANDLER_LPUARTx).
/// Return false if the fifo is empty.
bool khal_lpuart_GetFifo(LPUART_MemMapPtr uart, char *c);



#endif  /* _khal_uart_H_ */


