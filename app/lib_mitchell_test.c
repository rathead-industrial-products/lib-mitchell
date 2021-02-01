/*******************************************************************************

    lib_mitchell_test.c - Run unit tests on lib_mitchell modules.

    COPYRIGHT NOTICE: (c) 2015 DDPA LLC
    All Rights Reserved

 ******************************************************************************/

#include "derivative.h"
#include "contract.h"
#include "printf-emb.h"
#include "exec.h"
#include "memory.h"
#include "monitor.h"


/***** Private functions *****/
static void   hdweInit(void);
static void   systInit(void);
static void  _hdweSIMInitKL25(void);
static void  _hdweMCGInitKL25(void);
static void  _hdweSIMInitK22(void);
static void  _hdweMCGInitK22(void);

/***** Unit tests *****/
int printf_emb_UNIT_TEST(void);
int exec_UNIT_TEST(void);
int dl_UNIT_TEST(void);
int obuf_UNIT_TEST(void);
int fifo_UNIT_TEST(void);
int monitor_UNIT_TEST(void);
int button_UNIT_TEST(void);
int sh_UNIT_TEST(void);
int bitvector_UNIT_TEST(void);
int pool_UNIT_TEST(void);

ASSERT_INIT;


int main(void) {

    hdweInit();

    volatile int test_result_failures = 0;
    test_result_failures += printf_emb_UNIT_TEST();
    test_result_failures += exec_UNIT_TEST();
    test_result_failures += dl_UNIT_TEST();
    test_result_failures += obuf_UNIT_TEST();
    test_result_failures += fifo_UNIT_TEST();
    test_result_failures += monitor_UNIT_TEST();
    test_result_failures += button_UNIT_TEST();
    test_result_failures += sh_UNIT_TEST();
    test_result_failures += bitvector_UNIT_TEST();
    test_result_failures += pool_UNIT_TEST();

    for (;;) { } // wait for debugger inspection

}



/*******************************************************************************

    void  hdweInit(void)

    Do the minimum initialization necessary to get the processor running.

 ******************************************************************************/
void  hdweInit(void) {

#if (__CORTEX_M == 0)

    _hdweSIMInitKL25();    // module clock enables
    _hdweMCGInitKL25();    // get clock up to speed

#else

    _hdweSIMInitK22();    // module clock enables
    _hdweMCGInitK22();    // get clock up to speed

#endif

    systInit();

}

/*******************************************************************************

    void  systickInit(void)

    Enable systick clocked by the CPU core clock to allow
    instruction level counting.

******************************************************************************/
static void  systInit(void) {

    SysTick->LOAD = -1;                    // value not important
    SysTick->VAL  = 0;
    SysTick->CTRL = 0x00000007;            // RSVD = 0x000 000
                                           // COUNTFLAG = 0
                                           // RSVD = 0x000 0
                                           // CLKSOURCE = 1
                                           // TICKINT = 1
                                           // ENABLE = 1
}

/*****************************************************************************/


#if (__CORTEX_M == 0)

static void  _hdweSIMInitKL25(void) {
}

static void  _hdweMCGInitKL25(void) {
}

#endif


#if (__CORTEX_M == 4)
/*******************************************************************************

  SIM Initialization - K22 (120 MHz)

  Clock the core at 120 MHz using the IRC48 internal oscillator. Drive CLKOUT
  with the 60 MHz flash clock.

 ******************************************************************************/
static void  _hdweSIMInitK22(void) {


    //SIM_SOPT1CFG = 0x00000000;    // RSVD           = 0000 0
    // Reset value                  // USSWE          = 0
                                    // UVSWE          = 0
                                    // URWE           = 0
                                    // RSVD           = 0x000000

    //SIM_SOPT1 = 0x80000000;       // USBREGEN       = 1
    // Reset value                  // USBSSTBY       = 0
                                    // USBVSTBY       = 0
                                    // RSVD           = 0 0000 0000
                                    // OSC32KSEL      = 00
                                    // OSC32KSEL      = 00
                                    // RSVD           = 0x0000

    SIM_SOPT2 = 0x00071040;         // RSVD           = 0000
                                    // LPUARTSRC      = 00
                                    // RSVD           = 00 0000 0
                                    // USBSRC         = 1
                                    // PLLFLLSEL      = 11      (IRC48)
                                    // RSVD           = 000
                                    // TRACECLKSEL    = 1
                                    // RSVD           = 0000
                                    // CLKOUTSEL      = 010     (Flash Clock)
                                    // RTCCLKOUTSEL   = 0
                                    // RSVD           = 0000

    //SIM_SOPT4 = 0x00000000;       // RSVD           = 00
    // Reset value                  // FTM0TRG1SRC    = 0
                                    // FTM0TRG0SRC    = 0
                                    // RSVD           = 0
                                    // FTM2CLKSEL     = 0
                                    // FTM1CLKSEL     = 0
                                    // FTM0CLKSEL     = 0
                                    // RSVD           = 0
                                    // FTM2CH1SRC     = 0
                                    // FTM2CH0SRC     = 00
                                    // FTM1CH0SRC     = 00
                                    // RSVD           = 00 0000 000
                                    // FTM2FLT0       = 0
                                    // RSVD           = 000
                                    // FTM1FLT0       = 0
                                    // RSVD           = 00
                                    // FTM0FLT1       = 0
                                    // FTM0FLT0       = 0

    //SIM_SOPT5 = 0x00000000;       // RSVD           = 0x000
    // Reset value                  // LPUSRT0RXSRC   = 00
                                    // RSVD           = 00 0x00
                                    // UART1RXSRC     = 00
                                    // UART1TXSRC     = 00
                                    // UART1RXSRC     = 00
                                    // UART0TXSRC     = 00

    //SIM_SOPT7 = 0x00000000;       // RSVD           = 0x0000
    // Reset value                  // ADC1ALTTRGEN   = 0
                                    // RSVD           = 00
                                    // ADC1PERTRGSEL  = 0
                                    // ADC1TRGSEL     = 0000
                                    // ADC0ALTTRGEN   = 0
                                    // RSVD           = 00
                                    // ADC0PRETRGSEL  = 0
                                    // ADC0TRGSEL     = 0000

    //SIM_SOPT8 = 0x00000000;       // RSVD           = 0x00
    // Reset value                  // FTM0CH7SRC     = 0
                                    // FTM0CH6SRC     = 0
                                    // FTM0CH5SRC     = 0
                                    // FTM0CH4SRC     = 0
                                    // FTM0CH3SRC     = 0
                                    // FTM0CH2SRC     = 0
                                    // FTM0CH1SRC     = 0
                                    // FTM0CH0SRC     = 0
                                    // RSVD           = 0x000 0
                                    // FTM2SYNCBIT    = 0
                                    // FTM1SYNCBIT    = 0
                                    // FTM0SYNCBIT    = 0


//  SIM_SDID = DEVICE IDENTIFICATION (Read Only)


    //SIM_SCGC4 = 0xf0100030;       // RSVD           = 0xf0 000
    // Reset value                  // VREF           = 1
                                    // CMP            = 0
                                    // USBOTG         = 0
                                    // RSVD           = 00 000
                                    // UART2          = 0
                                    // UART1          = 0
                                    // UART0          = 0
                                    // RSVD           = 00
                                    // I2C1           = 0
                                    // I2C0           = 0
                                    // RSVD           = 11 00
                                    // EWM            = 0
                                    // RSVD           = 0

    //SIM_SCGC5 = 0x00040182;       // RSVD           = 0x0004 00
    // Reset value                  // PORTE          = 0
                                    // PORTD          = 0
                                    // PORTC          = 0
                                    // PORTB          = 0
                                    // PORTA          = 0
                                    // RSVD           = 1 1000 001
                                    // LPTMR          = 0

    //SIM_SCGC6 = 0x40000001;       // DAC0           = 0
    // Reset value                  // RSVD           = 1
                                    // RTC            = 0
                                    // RSVD           = 0
                                    // ADC0           = 0
                                    // FTM2           = 0
                                    // FTM1           = 0
                                    // FTM0           = 0
                                    // PIT            = 0
                                    // PDB            = 0
                                    // RSVD           = 00 0
                                    // CRC            = 0
                                    // RSVD           = 00
                                    // I2S            = 0
                                    // RSVD           = 0
                                    // SPI1           = 0
                                    // SPI0           = 0
                                    // RSVD           = 0
                                    // LPUART0        = 0
                                    // RNGA           = 0
                                    // RSVD           = 0
                                    // ADC1           = 0
                                    // RSVD           = 000 00
                                    // DMAMUX         = 0
                                    // FTF            = 1

    //SIM_SCGC7 = 0x00000020;       // RSVD           = 0x00000000 00
    // Reset value                  // DMA            = 1
                                    // RSVD           = 0

    //SIM_CLKDIV1 = 0x01040000;     // OUTDIV1        = 0000    (core clock:  divide-by-1)
    //Set in hdweMCGInit            // OUTDIV2        = 0001    (bus clock:   divide-by-2)
                                    // RSVD           = 0000
                                    // OUTDIV4        = 0100    (flash clock: divide-by-5)
                                    // RSVD           = 0x0000

    // SIM_CLKDIV2 = 0x00000000;    // RSVD           = 0x0000000
    // Reset value                  // USBDIV         = 000
                                    // USBFRAC        = 0

    // SIM_FCFG1 = 0x00000000;      // RSVD           = 0x0000000 00
    // Reset value                  // FLASHDOZE      = 0
                                    // FLASHDIS       = 0
    //  SIM_FCFG2 = (Read Only)

    //  SIM_UIDxxx = UNIQUE IDENTIFICATION REGISTER (Read Only - 96 bits)

}

/*******************************************************************************

    MCG and OSC Initialization - K22

    The MCG is configured to create a 48 MHz USB compliant system clock.

    The input reference is the IRC48 internal oscillator. The PLL will multiply
    this up to 120 MHz. The Core clock is 120 MHz, the Bus Clock is 60 MHz,
    and the Flash Clock is 24 MHz.

    At 120MHz, the System Mode Controller must be configured for High-Speed
    Run Mode.

 ******************************************************************************/
static void  _hdweMCGInitK22(void) {

    SMC_PMPROT = SMC_PMPROT_AHSRUN_MASK;  /* Allow HSRUN mode */
    SMC_PMCTRL = SMC_PMCTRL_RUNM(3);      /* Enable HSRUN mode */
    while(SMC_PMSTAT != 0x80U) { }        /* Wait until the system is in HSRUN mode */

    /*
     * At reset, mode is FLL Engaged Internal with the slow internal clock
     * as the reference and an FLL multiplier of 640.
     * MCGOUTCLK = 21 MHz (32 KHz x 640)
     */

    /* Select IRC48 as external source OSCCLK1 */
    MCG_C7  = 2;

    /*
     * A transition through FBE mode is required:
     *  Set Osc range to anything but zero so that FRDIV will divide by an additional 2^5
     *  Set FRDIV = 1280 to get IRC48 into 30-40 KHz range (48 MHz / 1280 = 37.5 KHz
     *  Select ext reference as FLL input
     *  Wait for FLL input to be recognized
     * MCGOUTCLK = 24 KHz (37.5 KHz x 640)
     *
     */
    MCG_C2 = (MCG_C1 & ~MCG_C2_RANGE0_MASK) | MCG_C2_RANGE0(2);
    MCG_C1 = (MCG_C1 & ~MCG_C1_FRDIV_MASK) | MCG_C1_FRDIV(6);
    MCG_C1 = (MCG_C1 & ~MCG_C1_IREFS_MASK);
    while (MCG_S & MCG_S_IREFST_MASK);    // spin until FLL ref clk is external (IREFST=0)

    /*
     * Configure the OUTDIV1, OUTDIV2, & OUTDIV4 clock dividers to divide by
     * 1, 2, & 5 respectively for Core/Bus/Flash clocks of 120, 60, & 24 MHz.
     *
     * First get running at 1/2 speed in RUN mode, switch to HSRUN, then
     * double the clocks. The clocks are only allowed to change a max of
     * a factor of two in HSRUN mode.
     */
    SIM_CLKDIV1 = 0x01040000;

    /* Set the PLL external reference divider to 24 for a 2 MHz reference clock (48 MHz / (23+1)) */
    MCG_C5 = (MCG_C5 &~MCG_C5_PRDIV0_MASK) | MCG_C5_PRDIV0(23);

    /* Set the PLL multiplier to 30 for a 60 MHz output clock (2 MHz x 30) */
    MCG_C6 =  MCG_C6_VDIV0(6);

    /* Select PLL Bypassed External mode */
    MCG_C6 =  MCG_C6 | MCG_C6_PLLS_MASK;

    /* Wait for the PLL to lock */
    while (!(MCG_S & MCG_S_LOCK0_MASK));

    /* Select PLL Engaged External mode */
    MCG_C1 = (MCG_C1 & ~MCG_C1_CLKS_MASK) | MCG_C1_CLKS(0);

    SMC_PMCTRL = SMC_PMCTRL_RUNM(3);      /* Enable HSRUN mode */
    while(SMC_PMSTAT != 0x80U) { }        /* Wait until the system is in HSRUN mode */

    /* Set the PLL external reference divider to 12 ( 4 MHz ext refclk) to double all the clocks to 120/60/24 MHz */
    MCG_C5 = (MCG_C5 &~MCG_C5_PRDIV0_MASK) | MCG_C5_PRDIV0(11);
}

#endif    /* (__CORTEX_M == 4) */

