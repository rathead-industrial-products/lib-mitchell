/**
 *
 *  Hardware Abstraction Layer for Kinetis UART modules.
 *
 *  COPYRIGHT NOTICE: (c) 2016 DDPA LLC
 *  All Rights Reserved
 *
 */


#include  <stdbool.h>
#include  <stdint.h>
#include  <string.h>
#include  "contract.h"
#include  "derivative.h"
#include  "khal_clk.h"
#include  "khal_port.h"
#include  "khal_uart.h"
#include  "memory.h"


/* local function definition */
static void _LPUART_IRQHandler(LPUART_MemMapPtr uart);

// ==== DEFINES ====

#ifndef LPUART0
    #define LPUART0        NULL
#endif

#ifndef LPUART1
    #define LPUART1        NULL
#endif

#ifdef KHAL_LPUART_INSTALL_IRQ_HANDLER_LPUART0
    NEW_FIFO(khal_lpuart_tx_buf_lpuart0, KHAL_LPUART_TX_FIFO_SIZE_LPUART0);
    NEW_FIFO(khal_lpuart_rx_buf_lpuart0, KHAL_LPUART_RX_FIFO_SIZE_LPUART0);
    #define khal_lpuart_tx_buf_lpuart0_ptr  &khal_lpuart_tx_buf_lpuart0
    #define khal_lpuart_rx_buf_lpuart0_ptr  &khal_lpuart_rx_buf_lpuart0
    void LPUART0_IRQHandler(void) { _LPUART_IRQHandler(LPUART0); }
#else
    #define khal_lpuart_tx_buf_lpuart0_ptr  (NULL)
    #define khal_lpuart_rx_buf_lpuart0_ptr  (NULL)
#endif  /* KHAL_LPUART_INSTALL_IRQ_HANDLER_LPUART0 */

#ifdef KHAL_LPUART_INSTALL_IRQ_HANDLER_LPUART1
    NEW_FIFO(khal_lpuart_tx_buf_lpuart1, KHAL_LPUART_TX_FIFO_SIZE_LPUART1);
    NEW_FIFO(khal_lpuart_rx_buf_lpuart1, KHAL_LPUART_RX_FIFO_SIZE_LPUART1);
    #define khal_lpuart_tx_buf_lpuart1_ptr  &khal_lpuart_tx_buf_lpuart1
    #define khal_lpuart_rx_buf_lpuart1_ptr  &khal_lpuart_rx_buf_lpuart1
    void LPUART0_IRQHandler(void) { _LPUART_IRQHandler(LPUART1); }
#else
    #define khal_lpuart_tx_buf_lpuart1_ptr  (NULL)
    #define khal_lpuart_rx_buf_lpuart1_ptr  (NULL)
#endif  /* KHAL_LPUART_INSTALL_IRQ_HANDLER_LPUART1 */

#define KHAL_LPUART_TX_BUF_PTR(lpuart)    (((lpuart) == LPUART0) ? khal_lpuart_tx_buf_lpuart0_ptr : (((lpuart) == LPUART1) ? khal_lpuart_tx_buf_lpuart1_ptr : NULL))
#define KHAL_LPUART_RX_BUF_PTR(lpuart)    (((lpuart) == LPUART0) ? khal_lpuart_rx_buf_lpuart0_ptr : (((lpuart) == LPUART1) ? khal_lpuart_rx_buf_lpuart1_ptr : NULL))


// ==== LPUART API ====

void khal_lpuart_Init(LPUART_MemMapPtr uart,
                      PORT_MemMapPtr tx_port, uint32_t tx_pin, uint32_t tx_mux,
                      PORT_MemMapPtr rx_port, uint32_t rx_pin, uint32_t rx_mux,
                      khal_lpuart_br_clk_src_t clk_src, uint32_t const sourceClockInHz,
                      uint32_t const desiredBaudRate) {

    khal_clk_ModuleClkGateEn(&SIM_SCGC5, 20, true);    // enable LPUART0 module
    khal_port_Init(tx_port, tx_pin, tx_mux,
                   KHAL_PORT_DATA_DIR_IN, 0,
                   KHAL_PORT_ATTR_PE_DIS, KHAL_PORT_ATTR_PS_UP,
                   KHAL_PORT_ATTR_SRE_SLOW, KHAL_PORT_ATTR_PFE_DIS,
                   KHAL_PORT_ATTR_ODE_DIS, KHAL_PORT_ATTR_DSE_LOW,
                   KHAL_PORT_INT_DMA_DIS);
    khal_port_Init(rx_port, rx_pin, rx_mux,
                   KHAL_PORT_DATA_DIR_IN, 0,
                   KHAL_PORT_ATTR_PE_DIS, KHAL_PORT_ATTR_PS_UP,
                   KHAL_PORT_ATTR_SRE_SLOW, KHAL_PORT_ATTR_PFE_DIS,
                   KHAL_PORT_ATTR_ODE_DIS, KHAL_PORT_ATTR_DSE_LOW,
                   KHAL_PORT_INT_DMA_DIS);
    khal_lpuart_SelectBRClk(uart, clk_src);
    khal_lpuart_SetBaudRate(uart, sourceClockInHz, desiredBaudRate);
    khal_lpuart_RxTxEn(uart, true);
}

void khal_lpuart_RxTxEn(LPUART_MemMapPtr uart, bool const enable) {
    uint32_t rxtxen = (enable) ? (LPUART_CTRL_TE_MASK | LPUART_CTRL_TE_MASK) : 0;
    uart->CTRL = (uart->CTRL & ~(LPUART_CTRL_TE_MASK | LPUART_CTRL_TE_MASK)) | rxtxen;
}

void khal_lpuart_IntEn(LPUART_MemMapPtr uart, uint32_t priority, bool const enable) {
    uint32_t tie = (enable) ? LPUART_CTRL_TIE_MASK : 0;
    uint32_t rie = (enable) ? LPUART_CTRL_RIE_MASK : 0;

    /// TODO: Support more than LPUART0
    if (enable) {
        NVIC_ClearPendingIRQ(LPUART0_IRQn);
        NVIC_SetPriority(LPUART0_IRQn, priority);
        NVIC_EnableIRQ(LPUART0_IRQn);
    }
    else {
        NVIC_DisableIRQ(LPUART0_IRQn);
    }

    uart->CTRL = (uart->CTRL & ~LPUART_CTRL_TIE_MASK) | tie;
    uart->CTRL = (uart->CTRL & ~LPUART_CTRL_RIE_MASK) | rie;
}

#if (defined MCU_MK22F25612)
    #define SIM_SOPT2_LPUART0SRC_MASK   SIM_SOPT2_LPUARTSRC_MASK
    #define SIM_SOPT2_LPUART0SRC(x)     SIM_SOPT2_LPUARTSRC(x)
#endif
void khal_lpuart_SelectBRClk(LPUART_MemMapPtr uart, khal_lpuart_br_clk_src_t clk_src) {
    REQUIRE (clk_src < KHAL_LPUART_BR_CLK_SRC_ERR);
    SIM_SOPT2 = (SIM_SOPT2 & ~SIM_SOPT2_LPUART0SRC_MASK) | SIM_SOPT2_LPUART0SRC(clk_src);
}

bool khal_lpuart_SetBaudRate(LPUART_MemMapPtr uart, uint32_t const sourceClockInHz, uint32_t const desiredBaudRate) {
    /*
     * BR  = CLK / (SBR X (OSR + 1))
     * SBR = CLK / (BR  X (OSR + 1))
     *
     * If baud rate cannot be set within 5% of desiredBaudRate
     * return false and leave SBR unchanged.
     */
    uint32_t osr1 = 1 + ((uart->BAUD & LPUART_BAUD_OSR_MASK) >> LPUART_BAUD_OSR_SHIFT);
    uint32_t sbr = (sourceClockInHz + ((desiredBaudRate * osr1) / 2)) / (desiredBaudRate * osr1); // compute rounded sbr
    uint32_t abr100 = 100 * (sourceClockInHz / (sbr * osr1));   // actual baud rate x 100 (to compute percentage)
    uint32_t err = abr100 / desiredBaudRate;
    if (err > 100) { err = err - 100; }   // error in percent
    else           { err = 100 - err; }
    if (err > 5) { return (false);  }
    else {
        uart->BAUD = (uart->BAUD & ~LPUART_BAUD_SBR_MASK) | sbr;
        return (true);
    }
}

bool khal_lpuart_TxIdle(LPUART_MemMapPtr uart) {
    return ((uart->STAT & LPUART_STAT_TC_MASK) == LPUART_STAT_TC_MASK);
}

bool khal_lpuart_TxBufEmpty(LPUART_MemMapPtr uart) {
    return ((uart->STAT & LPUART_STAT_TDRE_MASK) == LPUART_STAT_TDRE_MASK);
}

bool khal_lpuart_RxBufFull(LPUART_MemMapPtr uart) {
    return ((uart->STAT & LPUART_STAT_RDRF_MASK) == LPUART_STAT_RDRF_MASK);
}

void khal_lpuart_PutChar(LPUART_MemMapPtr uart, char data) {
    uart->DATA = (uint32_t) data;
}

char khal_lpuart_GetChar(LPUART_MemMapPtr uart) {
    return ((char) (uart->DATA & 0x000000ff));
}

bool khal_lpuart_PutFifo(LPUART_MemMapPtr uart, char *str) {
    bool rslt= true;

    if (strlen(str) > fifoRemaining(KHAL_LPUART_TX_BUF_PTR(uart))) { return (false); }

    while (*str) {
        if (uart->STAT & LPUART_STAT_TC_MASK) {
            uart->DATA = (uint32_t) *str;
            uart->CTRL |= LPUART_CTRL_TIE_MASK;
        }
        else {
            rslt &= fifoPush((TFifo) KHAL_LPUART_TX_BUF_PTR(uart), *str);
        }
        ++str;
    }
    return (rslt);
}

bool khal_lpuart_GetFifo(LPUART_MemMapPtr uart, char *c) {
    return (fifoPop((TFifo) KHAL_LPUART_RX_BUF_PTR(uart), c));
}

static void _LPUART_IRQHandler(LPUART_MemMapPtr uart) {
    char c;

    if (uart->STAT & LPUART_STAT_TDRE_MASK) {
        if (fifoPop((TFifo) KHAL_LPUART_TX_BUF_PTR(uart), &c) ){
            uart->DATA = (uint32_t) c;
        }
        else {
            uart->CTRL &= ~LPUART_CTRL_TIE_MASK;
        }
    }
    if (uart->STAT & LPUART_STAT_RDRF_MASK) {
        c = (char) (uart->DATA & 0x000000ff);
        fifoPush((TFifo) KHAL_LPUART_RX_BUF_PTR(uart), c);
    }
}






