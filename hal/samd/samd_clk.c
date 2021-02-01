/**
 *
 *  Hardware Abstraction Layer for SAMD MCU clock and oscillator modules.
 *
 *  COPYRIGHT NOTICE: (c) 2018 DDPA LLC
 *  All Rights Reserved
 *
 */


#include  <stdbool.h>
#include  <stdint.h>
#include  <sam.h>
#include  "samd_clk.h"
#include  "samd_port.h"

//#define CRYSTALLESS     // define if XOSC32K is not available


// ==== EXTERNAL CLOCK OUT API ====

/*******************************************************************************

    void  _gclk_out(bool enable, uint32_t gclk_sel, samd_port_pin_t port_pin)

    Configure an external pin to drive a selcted gclk.

******************************************************************************/
void _gclk_out(bool enable, uint32_t gclk_sel, samd_port_pin_t port_pin) {

    if (enable) {
        // enable gclk OE
        #if   defined(__SAMD11__)
        GCLK->GENCTRL.bit.ID = gclk_sel;
        while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) {}
        GCLK->GENCTRL.reg |= GCLK_GENCTRL_OE;
        while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) {}
        samd_port_AltFunc(port_pin, SAMD_PORT_PERIPHERAL_FUNC_H);

        #elif defined(__SAMD51__)
        GCLK->GENCTRL[gclk_sel].bit.OE = 1;
        while (GCLK->SYNCBUSY.vec.GENCTRL) {}
        samd_port_AltFunc(port_pin, SAMD_PORT_PERIPHERAL_FUNC_M);
        #endif

        SAMD_PORT_PIN_OUTPUT_ENABLE(port_pin);
    }

    else {
        // disable gclk OE
        #ifdef __SAMD11__
        GCLK->GENCTRL.bit.ID = gclk_sel;
        while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) {}
        GCLK->GENCTRL.reg &= ~GCLK_GENCTRL_OE;
        while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) {}

        #elif defined(__SAMD51__)
        GCLK->GENCTRL[gclk_sel].bit.OE = 0;
        while (GCLK->SYNCBUSY.vec.GENCTRL) {}
        #endif
        // reset pin to default condition
        samd_port_Disable(port_pin);
    }
}

void samd11_glck0OutPA24(bool enable) { _gclk_out(enable, 0, SAMD_PORT_PIN(SAMD_PORTA, 24)); }
void samd11_glck1OutPA22(bool enable) { _gclk_out(enable, 1, SAMD_PORT_PIN(SAMD_PORTA, 22)); }    // not tested
void samd11_glck2OutPA16(bool enable) { _gclk_out(enable, 2, SAMD_PORT_PIN(SAMD_PORTA, 16)); }    // not tested
void samd11_glck4OutPA14(bool enable) { _gclk_out(enable, 4, SAMD_PORT_PIN(SAMD_PORTA, 14)); }    // not tested
void samd11_glck5OutPA15(bool enable) { _gclk_out(enable, 5, SAMD_PORT_PIN(SAMD_PORTA, 15)); }    // not tested

void samd51_glck0OutPA14(bool enable) { _gclk_out(enable, 0, SAMD_PORT_PIN(SAMD_PORTA, 14)); }
void samd51_glck1OutPB23(bool enable) { _gclk_out(enable, 1, SAMD_PORT_PIN(SAMD_PORTB, 23)); }
void samd51_glck2OutPA16(bool enable) { _gclk_out(enable, 2, SAMD_PORT_PIN(SAMD_PORTA, 16)); }
void samd51_glck3OutPB17(bool enable) { _gclk_out(enable, 3, SAMD_PORT_PIN(SAMD_PORTB, 17)); }

// ==== GCLK CONFIGURATION API ====




/*******************************************************************************
 *
 * System Clock Initialization copied verbatum from Arduino SystemInit().
 *
 * SAMD21
 *    GCLK 0 (GCLK MAIN): 48 MHz, locked to GCLK 1
 *    GLCK 1: 32.768 KHz crystal
 *    GCLK 2: OSCULP32K
 *    GCLK 3: OSC8M
 *    GCLK 4-8:
 *    Also sets up ADC calibration and disables automatic NVM write operations.
 *
 * SAMD51
 *    GCLK 0 (GCLK MAIN): 120 MHz, locked to GCLK TDB
 *    GLCK 1: 48 MHz, locked to GCLK 3
 *    GCLK 2: 100 MHz, locked to TBD
 *    GCLK 3: XOSC32K
 *    GCLK 8: 12 MHz, locked to TBD
 *    GCLK 5-11:
 *
 */


// Constants for Clock generators
#define GENERIC_CLOCK_GENERATOR_MAIN      (0u)


//***************** SAMD51 ************************//

#if defined(__SAMD51__)

#define GENERIC_CLOCK_GENERATOR_XOSC32K   (3u)
#define GENERIC_CLOCK_GENERATOR_48M     (1u)
#define GENERIC_CLOCK_GENERATOR_48M_SYNC  GCLK_SYNCBUSY_GENCTRL1
#define GENERIC_CLOCK_GENERATOR_100M    (2u)
#define GENERIC_CLOCK_GENERATOR_100M_SYNC GCLK_SYNCBUSY_GENCTRL2
#define GENERIC_CLOCK_GENERATOR_12M       (4u)
#define GENERIC_CLOCK_GENERATOR_12M_SYNC   GCLK_SYNCBUSY_GENCTRL4

//USE DPLL0 for 120MHZ
#define MAIN_CLOCK_SOURCE         GCLK_GENCTRL_SRC_DPLL0
#define GENERIC_CLOCK_GENERATOR_1M      (7u)

void samd_clkInit( void ) {

  NVMCTRL->CTRLA.reg |= NVMCTRL_CTRLA_RWS(0);

  #ifndef CRYSTALLESS
  /* ----------------------------------------------------------------------------------------------
   * 1) Enable XOSC32K clock (External on-board 32.768Hz oscillator)
   */

  OSC32KCTRL->XOSC32K.reg = OSC32KCTRL_XOSC32K_ENABLE | OSC32KCTRL_XOSC32K_EN32K | OSC32KCTRL_XOSC32K_EN32K | OSC32KCTRL_XOSC32K_CGM_XT | OSC32KCTRL_XOSC32K_XTALEN;

  while( (OSC32KCTRL->STATUS.reg & OSC32KCTRL_STATUS_XOSC32KRDY) == 0 ){
    /* Wait for oscillator to be ready */
  }

  #endif //CRYSTALLESS

  //software reset

  GCLK->CTRLA.bit.SWRST = 1;
  while ( GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_SWRST ){
    /* wait for reset to complete */
  }

  #ifndef CRYSTALLESS
  /* ----------------------------------------------------------------------------------------------
   * 2) Put XOSC32K as source of Generic Clock Generator 3
   */
  GCLK->GENCTRL[GENERIC_CLOCK_GENERATOR_XOSC32K].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_XOSC32K) | //generic clock gen 3
    GCLK_GENCTRL_GENEN;
  #else
  /* ----------------------------------------------------------------------------------------------
   * 2) Put OSCULP32K as source of Generic Clock Generator 3
   */
  GCLK->GENCTRL[GENERIC_CLOCK_GENERATOR_XOSC32K].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_OSCULP32K) | GCLK_GENCTRL_GENEN; //generic clock gen 3
  #endif


  while ( GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL3 ){
    /* Wait for synchronization */
  }

  /* ----------------------------------------------------------------------------------------------
   * 3) Put Generic Clock Generator 3 as source for Generic Clock Gen 0 (DFLL48M reference)
   */
  GCLK->GENCTRL[0].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_OSCULP32K) | GCLK_GENCTRL_GENEN;

  /* ----------------------------------------------------------------------------------------------
   * 4) Enable DFLL48M clock
   */

  while ( GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL0 ){
    /* Wait for synchronization */
  }

  /* DFLL Configuration in Open Loop mode */

  OSCCTRL->DFLLCTRLA.reg = 0;
  //GCLK->PCHCTRL[OSCCTRL_GCLK_ID_DFLL48].reg = (1 << GCLK_PCHCTRL_CHEN_Pos) | GCLK_PCHCTRL_GEN(GCLK_PCHCTRL_GEN_GCLK3_Val);

  OSCCTRL->DFLLMUL.reg = OSCCTRL_DFLLMUL_CSTEP( 0x1 ) |
    OSCCTRL_DFLLMUL_FSTEP( 0x1 ) |
    OSCCTRL_DFLLMUL_MUL( 0 );

  while ( OSCCTRL->DFLLSYNC.reg & OSCCTRL_DFLLSYNC_DFLLMUL )
    {
      /* Wait for synchronization */
    }

  OSCCTRL->DFLLCTRLB.reg = 0;
  while ( OSCCTRL->DFLLSYNC.reg & OSCCTRL_DFLLSYNC_DFLLCTRLB )
    {
      /* Wait for synchronization */
    }

  OSCCTRL->DFLLCTRLA.reg |= OSCCTRL_DFLLCTRLA_ENABLE;
  while ( OSCCTRL->DFLLSYNC.reg & OSCCTRL_DFLLSYNC_ENABLE )
    {
      /* Wait for synchronization */
    }

  OSCCTRL->DFLLVAL.reg = OSCCTRL->DFLLVAL.reg;
  while( OSCCTRL->DFLLSYNC.bit.DFLLVAL );

  OSCCTRL->DFLLCTRLB.reg = OSCCTRL_DFLLCTRLB_WAITLOCK |
  OSCCTRL_DFLLCTRLB_CCDIS | OSCCTRL_DFLLCTRLB_USBCRM ;

  while ( !OSCCTRL->STATUS.bit.DFLLRDY )
    {
      /* Wait for synchronization */
    }

  GCLK->GENCTRL[GENERIC_CLOCK_GENERATOR_1M].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_DFLL_Val) | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_DIV(24u);

  while ( GCLK->SYNCBUSY.bit.GENCTRL5 ){
    /* Wait for synchronization */
  }


  /* ------------------------------------------------------------------------
  * Set up the PLLs
  */

  //PLL0 is 120MHz
  GCLK->PCHCTRL[OSCCTRL_GCLK_ID_FDPLL0].reg = (1 << GCLK_PCHCTRL_CHEN_Pos) | GCLK_PCHCTRL_GEN(GCLK_PCHCTRL_GEN_GCLK7_Val);

  OSCCTRL->Dpll[0].DPLLRATIO.reg = OSCCTRL_DPLLRATIO_LDRFRAC(0x00) | OSCCTRL_DPLLRATIO_LDR(59); //120 Mhz

  while(OSCCTRL->Dpll[0].DPLLSYNCBUSY.bit.DPLLRATIO);

  //MUST USE LBYPASS DUE TO BUG IN REV A OF SAMD51
  OSCCTRL->Dpll[0].DPLLCTRLB.reg = OSCCTRL_DPLLCTRLB_REFCLK_GCLK | OSCCTRL_DPLLCTRLB_LBYPASS;

  OSCCTRL->Dpll[0].DPLLCTRLA.reg = OSCCTRL_DPLLCTRLA_ENABLE;

  while( OSCCTRL->Dpll[0].DPLLSTATUS.bit.CLKRDY == 0 || OSCCTRL->Dpll[0].DPLLSTATUS.bit.LOCK == 0 );

  //PLL1 is 100MHz
  GCLK->PCHCTRL[OSCCTRL_GCLK_ID_FDPLL1].reg = (1 << GCLK_PCHCTRL_CHEN_Pos) | GCLK_PCHCTRL_GEN(GCLK_PCHCTRL_GEN_GCLK7_Val);

  OSCCTRL->Dpll[1].DPLLRATIO.reg = OSCCTRL_DPLLRATIO_LDRFRAC(0x00) | OSCCTRL_DPLLRATIO_LDR(49); //100 Mhz

  while(OSCCTRL->Dpll[1].DPLLSYNCBUSY.bit.DPLLRATIO);

  //MUST USE LBYPASS DUE TO BUG IN REV A OF SAMD51
  OSCCTRL->Dpll[1].DPLLCTRLB.reg = OSCCTRL_DPLLCTRLB_REFCLK_GCLK | OSCCTRL_DPLLCTRLB_LBYPASS;

  OSCCTRL->Dpll[1].DPLLCTRLA.reg = OSCCTRL_DPLLCTRLA_ENABLE;

  while( OSCCTRL->Dpll[1].DPLLSTATUS.bit.CLKRDY == 0 || OSCCTRL->Dpll[1].DPLLSTATUS.bit.LOCK == 0 );


  /* ------------------------------------------------------------------------
  * Set up the peripheral clocks
  */

  //48MHZ CLOCK FOR USB AND STUFF
  GCLK->GENCTRL[GENERIC_CLOCK_GENERATOR_48M].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_DFLL_Val) |
    GCLK_GENCTRL_IDC |
    //GCLK_GENCTRL_OE |
    GCLK_GENCTRL_GENEN;

  while ( GCLK->SYNCBUSY.reg & GENERIC_CLOCK_GENERATOR_48M_SYNC)
    {
      /* Wait for synchronization */
    }

  //100MHZ CLOCK FOR OTHER PERIPHERALS
  GCLK->GENCTRL[GENERIC_CLOCK_GENERATOR_100M].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_DPLL1_Val) |
    GCLK_GENCTRL_IDC |
    //GCLK_GENCTRL_OE |
    GCLK_GENCTRL_GENEN;

  while ( GCLK->SYNCBUSY.reg & GENERIC_CLOCK_GENERATOR_100M_SYNC)
    {
      /* Wait for synchronization */
    }

  //12MHZ CLOCK FOR DAC
  GCLK->GENCTRL[GENERIC_CLOCK_GENERATOR_12M].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_DFLL_Val) |
    GCLK_GENCTRL_IDC |
    GCLK_GENCTRL_DIV(4) |
    GCLK_GENCTRL_DIVSEL |
    //GCLK_GENCTRL_OE |
    GCLK_GENCTRL_GENEN;

  while ( GCLK->SYNCBUSY.reg & GENERIC_CLOCK_GENERATOR_12M_SYNC)
    {
      /* Wait for synchronization */
    }

  /*---------------------------------------------------------------------
   * Set up main clock
   */

  GCLK->GENCTRL[GENERIC_CLOCK_GENERATOR_MAIN].reg = GCLK_GENCTRL_SRC(MAIN_CLOCK_SOURCE) |
    GCLK_GENCTRL_IDC |
    //GCLK_GENCTRL_OE |
    GCLK_GENCTRL_GENEN;


  while ( GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL0 )
    {
      /* Wait for synchronization */
    }

  MCLK->CPUDIV.reg = MCLK_CPUDIV_DIV_DIV1;

  /* Use the LDO regulator by default */
  SUPC->VREG.bit.SEL = 0;


  /* If desired, enable cache! */
#if defined(ENABLE_CACHE)
  __disable_irq();
  CMCC->CTRL.reg = 1;
  __enable_irq();
#endif

}

//********************* END SAMD51 ********************************//

#else

//********************** SAMD21, SAMD09/10/11 *********************//

  /**
 * \brief SystemInit() configures the needed clocks and according Flash Read Wait States.
 * At reset:
 * - OSC8M clock source is enabled with a divider by 8 (1MHz).
 * - Generic Clock Generator 0 (GCLKMAIN) is using OSC8M as source.
 * We need to:
 * 1) Enable XOSC32K clock (External on-board 32.768Hz oscillator), will be used as DFLL48M reference.
 * 2) Put XOSC32K as source of Generic Clock Generator 1
 * 3) Put Generic Clock Generator 1 as source for Generic Clock Multiplexer 0 (DFLL48M reference)
 * 4) Enable DFLL48M clock
 * 5) Switch Generic Clock Generator 0 to DFLL48M. CPU will run at 48MHz.
 * 6) Modify PRESCaler value of OSCM to have 8MHz
 * 7) Put OSC8M as source for Generic Clock Generator 3
 */

#define GENERIC_CLOCK_GENERATOR_XOSC32K   (1u)
#define GENERIC_CLOCK_GENERATOR_OSC32K    (1u)
#define GENERIC_CLOCK_GENERATOR_OSCULP32K (2u) /* Initialized at reset for WDT */
#define GENERIC_CLOCK_GENERATOR_OSC8M     (3u)
// Constants for Clock multiplexers
#define GENERIC_CLOCK_MULTIPLEXER_DFLL48M (0u)
// Frequency of the board main oscillator
#define VARIANT_MAINOSC (32768ul)
// Master clock frequency
#define VARIANT_MCK     (48000000ul)

void samd_clkInit( void ) {

  /* Set 1 Flash Wait State for 48MHz, cf tables 20.9 and 35.27 in SAMD21 Datasheet */
    NVMCTRL->CTRLB.bit.RWS = NVMCTRL_CTRLB_RWS_HALF_Val ;

    /* Turn on the digital interface clock */
    PM->APBAMASK.reg |= PM_APBAMASK_GCLK ;


  #if defined(CRYSTALLESS)

    /* ----------------------------------------------------------------------------------------------
     * 1) Enable OSC32K clock (Internal 32.768Hz oscillator)
     */

    uint32_t calib = (*((uint32_t *) FUSES_OSC32K_CAL_ADDR) & FUSES_OSC32K_CAL_Msk) >> FUSES_OSC32K_CAL_Pos;

    SYSCTRL->OSC32K.reg = SYSCTRL_OSC32K_CALIB(calib) |
                          SYSCTRL_OSC32K_STARTUP( 0x6u ) | // cf table 15.10 of product datasheet in chapter 15.8.6
                          SYSCTRL_OSC32K_EN32K |
                          SYSCTRL_OSC32K_ENABLE;

    while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_OSC32KRDY) == 0 ); // Wait for oscillator stabilization

  #else // has crystal

    /* ----------------------------------------------------------------------------------------------
     * 1) Enable XOSC32K clock (External on-board 32.768Hz oscillator)
     */
    SYSCTRL->XOSC32K.reg = SYSCTRL_XOSC32K_STARTUP( 0x6u ) | /* cf table 15.10 of product datasheet in chapter 15.8.6 */
                           SYSCTRL_XOSC32K_XTALEN | SYSCTRL_XOSC32K_EN32K ;
    SYSCTRL->XOSC32K.bit.ENABLE = 1 ; /* separate call, as described in chapter 15.6.3 */

    while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_XOSC32KRDY) == 0 )
    {
      /* Wait for oscillator stabilization */
    }

  #endif

    /* Software reset the module to ensure it is re-initialized correctly */
    /* Note: Due to synchronization, there is a delay from writing CTRL.SWRST until the reset is complete.
     * CTRL.SWRST and STATUS.SYNCBUSY will both be cleared when the reset is complete, as described in chapter 13.8.1
     */
    GCLK->CTRL.reg = GCLK_CTRL_SWRST ;

    while ( (GCLK->CTRL.reg & GCLK_CTRL_SWRST) && (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) )
    {
      /* Wait for reset to complete */
    }

    /* ----------------------------------------------------------------------------------------------
     * 2) Put XOSC32K as source of Generic Clock Generator 1
     */
    GCLK->GENDIV.reg = GCLK_GENDIV_ID( GENERIC_CLOCK_GENERATOR_XOSC32K ) ; // Generic Clock Generator 1

    while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
    {
      /* Wait for synchronization */
    }

    /* Write Generic Clock Generator 1 configuration */
    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID( GENERIC_CLOCK_GENERATOR_OSC32K ) | // Generic Clock Generator 1
  #if defined(CRYSTALLESS)
                        GCLK_GENCTRL_SRC_OSC32K | // Selected source is Internal 32KHz Oscillator
  #else
                        GCLK_GENCTRL_SRC_XOSC32K | // Selected source is External 32KHz Oscillator
  #endif
  //                      GCLK_GENCTRL_OE | // Output clock to a pin for tests
                        GCLK_GENCTRL_GENEN ;

    while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
    {
      /* Wait for synchronization */
    }

    /* ----------------------------------------------------------------------------------------------
     * 3) Put Generic Clock Generator 1 as source for Generic Clock Multiplexer 0 (DFLL48M reference)
     */
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( GENERIC_CLOCK_MULTIPLEXER_DFLL48M ) | // Generic Clock Multiplexer 0
                        GCLK_CLKCTRL_GEN_GCLK1 | // Generic Clock Generator 1 is source
                        GCLK_CLKCTRL_CLKEN ;

    while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
    {
      /* Wait for synchronization */
    }

    /* ----------------------------------------------------------------------------------------------
     * 4) Enable DFLL48M clock
     */

    /* DFLL Configuration in Closed Loop mode, cf product datasheet chapter 15.6.7.1 - Closed-Loop Operation */

    /* Remove the OnDemand mode, Bug http://avr32.icgroup.norway.atmel.com/bugzilla/show_bug.cgi?id=9905 */
    SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_ENABLE;

    while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )
    {
      /* Wait for synchronization */
    }

    SYSCTRL->DFLLMUL.reg = SYSCTRL_DFLLMUL_CSTEP( 31 ) | // Coarse step is 31, half of the max value
                           SYSCTRL_DFLLMUL_FSTEP( 511 ) | // Fine step is 511, half of the max value
                           SYSCTRL_DFLLMUL_MUL( (VARIANT_MCK + VARIANT_MAINOSC/2) / VARIANT_MAINOSC ) ; // External 32KHz is the reference

    while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )
    {
      /* Wait for synchronization */
    }

  #if defined(CRYSTALLESS)

    #define NVM_SW_CALIB_DFLL48M_COARSE_VAL 58

    // Turn on DFLL
    uint32_t coarse =( *((uint32_t *)(NVMCTRL_OTP4) + (NVM_SW_CALIB_DFLL48M_COARSE_VAL / 32)) >> (NVM_SW_CALIB_DFLL48M_COARSE_VAL % 32) )
                     & ((1 << 6) - 1);
    if (coarse == 0x3f) {
      coarse = 0x1f;
    }
    // TODO(tannewt): Load this value from memory we've written previously. There
    // isn't a value from the Atmel factory.
    uint32_t fine = 0x1ff;

    SYSCTRL->DFLLVAL.bit.COARSE = coarse;
    SYSCTRL->DFLLVAL.bit.FINE = fine;
    /* Write full configuration to DFLL control register */
    SYSCTRL->DFLLMUL.reg = SYSCTRL_DFLLMUL_CSTEP( 0x1f / 4 ) | // Coarse step is 31, half of the max value
                           SYSCTRL_DFLLMUL_FSTEP( 10 ) |
                           SYSCTRL_DFLLMUL_MUL( (48000) ) ;

    SYSCTRL->DFLLCTRL.reg = 0;

    while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )
    {
      /* Wait for synchronization */
    }

    SYSCTRL->DFLLCTRL.reg =  SYSCTRL_DFLLCTRL_MODE |
                             SYSCTRL_DFLLCTRL_CCDIS |
                             SYSCTRL_DFLLCTRL_USBCRM | /* USB correction */
                             SYSCTRL_DFLLCTRL_BPLCKC;

    while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )
    {
      /* Wait for synchronization */
    }

    /* Enable the DFLL */
    SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_ENABLE ;

  #else   // has crystal

    /* Write full configuration to DFLL control register */
    SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_MODE | /* Enable the closed loop mode */
                             SYSCTRL_DFLLCTRL_WAITLOCK |
                             SYSCTRL_DFLLCTRL_QLDIS ; /* Disable Quick lock */

    while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )
    {
      /* Wait for synchronization */
    }

    /* Enable the DFLL */
    SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_ENABLE ;

    while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKC) == 0 ||
            (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKF) == 0 )
    {
      /* Wait for locks flags */
    }

  #endif

    while ( (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0 )
    {
      /* Wait for synchronization */
    }

    /* ----------------------------------------------------------------------------------------------
     * 5) Switch Generic Clock Generator 0 to DFLL48M. CPU will run at 48MHz.
     */
    GCLK->GENDIV.reg = GCLK_GENDIV_ID( GENERIC_CLOCK_GENERATOR_MAIN ) ; // Generic Clock Generator 0

    while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
    {
      /* Wait for synchronization */
    }

    /* Write Generic Clock Generator 0 configuration */
    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID( GENERIC_CLOCK_GENERATOR_MAIN ) | // Generic Clock Generator 0
                        GCLK_GENCTRL_SRC_DFLL48M | // Selected source is DFLL 48MHz
  //                      GCLK_GENCTRL_OE | // Output clock to a pin for tests
                        GCLK_GENCTRL_IDC | // Set 50/50 duty cycle
                        GCLK_GENCTRL_GENEN ;

    while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
    {
      /* Wait for synchronization */
    }

    /* ----------------------------------------------------------------------------------------------
     * 6) Modify PRESCaler value of OSC8M to have 8MHz
     */
    SYSCTRL->OSC8M.bit.PRESC = SYSCTRL_OSC8M_PRESC_0_Val ;  //CMSIS 4.5 changed the prescaler defines
    SYSCTRL->OSC8M.bit.ONDEMAND = 0 ;

    /* ----------------------------------------------------------------------------------------------
     * 7) Put OSC8M as source for Generic Clock Generator 3
     */
    GCLK->GENDIV.reg = GCLK_GENDIV_ID( GENERIC_CLOCK_GENERATOR_OSC8M ) ; // Generic Clock Generator 3

    /* Write Generic Clock Generator 3 configuration */
    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID( GENERIC_CLOCK_GENERATOR_OSC8M ) | // Generic Clock Generator 3
                        GCLK_GENCTRL_SRC_OSC8M | // Selected source is RC OSC 8MHz (already enabled at reset)
  //                      GCLK_GENCTRL_OE | // Output clock to a pin for tests
                        GCLK_GENCTRL_GENEN ;

    while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY )
    {
      /* Wait for synchronization */
    }

    /*
     * Now that all system clocks are configured, we can set CPU and APBx BUS clocks.
     * There values are normally the one present after Reset.
     */
    PM->CPUSEL.reg  = PM_CPUSEL_CPUDIV_DIV1 ;
    PM->APBASEL.reg = PM_APBASEL_APBADIV_DIV1_Val ;
    PM->APBBSEL.reg = PM_APBBSEL_APBBDIV_DIV1_Val ;
    PM->APBCSEL.reg = PM_APBCSEL_APBCDIV_DIV1_Val ;

    //SystemCoreClock=VARIANT_MCK ;

    /* ----------------------------------------------------------------------------------------------
     * 8) Load ADC factory calibration values
     */

    // ADC Bias Calibration
    uint32_t bias = (*((uint32_t *) ADC_FUSES_BIASCAL_ADDR) & ADC_FUSES_BIASCAL_Msk) >> ADC_FUSES_BIASCAL_Pos;

    // ADC Linearity bits 4:0
    uint32_t linearity = (*((uint32_t *) ADC_FUSES_LINEARITY_0_ADDR) & ADC_FUSES_LINEARITY_0_Msk) >> ADC_FUSES_LINEARITY_0_Pos;

    // ADC Linearity bits 7:5
    linearity |= ((*((uint32_t *) ADC_FUSES_LINEARITY_1_ADDR) & ADC_FUSES_LINEARITY_1_Msk) >> ADC_FUSES_LINEARITY_1_Pos) << 5;

    ADC->CALIB.reg = ADC_CALIB_BIAS_CAL(bias) | ADC_CALIB_LINEARITY_CAL(linearity);

    /*
     * 9) Disable automatic NVM write operations
     */
    NVMCTRL->CTRLB.bit.MANW = 1;
}


/*******************************************************************************

    void _configSystick(uint8_t source_clock)

    Set SysTick to generate 1 ms interrupts at src_clk_freq frequency,
    except when src_clk_freq is 32 Khz, then interrupts are at 10 ms intervals.

******************************************************************************/
void _configSystick(uint32_t src_clk_freq) {
    SysTick->LOAD  = (src_clk_freq/1000) - 1;
    SysTick->VAL   = 0;
}


/*******************************************************************************

    void _configGCLK0(uint8_t source_clock, uint32_t division_factor)

    Set Generic Clock Generator 0 to default values with source_clock
    divided by division_factor.

    Default configuration values
    config->division_factor    = 1;
    config->high_when_disabled = false;
    config->source_clock       = GCLK_SOURCE_OSC8M;
    config->run_in_standby     = false;
    config->output_enable      = false;

******************************************************************************/
struct system_gclk_gen_config {
  uint8_t   source_clock;
  bool      high_when_disabled;
  uint32_t  division_factor;
  bool      run_in_standby;
  bool      output_enable;
};

void _configGCLK0(uint8_t source_clock, uint32_t division_factor) {
    struct system_gclk_gen_config config = { source_clock, false, division_factor, false, false };
    const uint8_t generator = 0;

    /*
     * Copied from gclk.c, part of SAM System Clock Management (SYSTEM CLOCK) Driver
     */

    /* Cache new register configurations to minimize sync requirements. */
    uint32_t new_genctrl_config = (generator << GCLK_GENCTRL_ID_Pos);
    uint32_t new_gendiv_config  = (generator << GCLK_GENDIV_ID_Pos);

    /* Select the requested source clock for the generator */
    new_genctrl_config |= config.source_clock << GCLK_GENCTRL_SRC_Pos;

    /* Configure the clock to be either high or low when disabled */
    if (config.high_when_disabled) {
    new_genctrl_config |= GCLK_GENCTRL_OOV;
    }

    /* Configure if the clock output to I/O pin should be enabled. */
    if (config.output_enable) {
    new_genctrl_config |= GCLK_GENCTRL_OE;
    }

    /* Set division factor */
    if (config.division_factor > 1) {
        /* Check if division is a power of two */
        if (((config.division_factor & (config.division_factor - 1)) == 0)) {
          /* Determine the index of the highest bit set to get the
           * division factor that must be loaded into the division
           * register */

          uint32_t div2_count = 0;

          uint32_t mask;
          for (mask = (1UL << 1); mask < config.division_factor;
                mask <<= 1) {
            div2_count++;
          }

          /* Set binary divider power of 2 division factor */
          new_gendiv_config  |= div2_count << GCLK_GENDIV_DIV_Pos;
          new_genctrl_config |= GCLK_GENCTRL_DIVSEL;
        } else {
          /* Set integer division factor */

          new_gendiv_config  |=
              (config.division_factor) << GCLK_GENDIV_DIV_Pos;

          /* Enable non-binary division with increased duty cycle accuracy */
          new_genctrl_config |= GCLK_GENCTRL_IDC;
        }
    }
}


/*******************************************************************************

    void _configGCLK_MAIN(uint8_t source_clock, uint32_t division_factor)

    Configure the CPU clock from the low frequency source_clk. Set flash
    wait states to 0 and disable DFLL48M.

******************************************************************************/
void _configGCLK_MAIN(uint8_t source_clock, uint32_t division_factor) {
    _configGCLK0(source_clock, 1);
    NVMCTRL->CTRLB.bit.RWS = NVMCTRL_CTRLB_RWS_SINGLE_Val;  // flash wait states = 0
    SYSCTRL->DFLLCTRL.reg  = 0;                             // disable DFLL48M
}


/*******************************************************************************

    void samd21_GCLK_MAIN_8M(void)

    Source Generic Clock Generator 0 from OSC8M.
    The CPU will now run at 8 MHz.
    Set flash wait-states to 0.
    Leave XOSC32K as source of Generic Clock Generator 1.
    Leave OSC8M as the source for Generic Clock Generator 3.
    Disable DFLL48M.
    Reconfigure SysTick for 8M clock.

******************************************************************************/
void samd21_GCLK_MAIN_8M(void) {
    _configGCLK_MAIN(GCLK_SOURCE_OSC8M, 1);
    _configSystick(8000000);
}


/*******************************************************************************

    void samd21_GCLK_MAIN_1M(void)

    Source Generic Clock Generator 0 from OSC8M divided by 8.
    The CPU will now run at 1 MHz.
    Set flash wait-states to 0.
    Leave XOSC32K as source of Generic Clock Generator 1.
    Leave OSC8M as the source for Generic Clock Generator 3.
    Disable DFLL48M.
    Reconfigure SysTick for 1M clock.

******************************************************************************/
void samd21_GCLK_MAIN_1M(void) {
    _configGCLK_MAIN(GCLK_SOURCE_OSC8M, 8);
    _configSystick(1000000);
}


/*******************************************************************************

    void samd21_GCLK_MAIN_32K(void)

    Source Generic Clock Generator 0 from XOSC32K.
    The CPU will now run at 32 KHz.
    Set flash wait-states to 0.
    Leave XOSC32K as source of Generic Clock Generator 1.
    Leave OSC8M as the source for Generic Clock Generator 3.
    Disable DFLL48M.
    Reconfigure SysTick for 32K clock - 10 ms tick intervals

******************************************************************************/
void samd21_GCLK_MAIN_32K(void) {
    _configGCLK_MAIN(GCLK_SOURCE_XOSC32K, 1);
    _configSystick(320000);
}


/*******************************************************************************

    void samd_GCLK_MAIN_48M_CL(void)

    Restores the oscillators and generic clock generators to their Arduino
    startup state. Copied from SystemInit() in startup.c.

    Reconfigure SysTick for 48M clock.

******************************************************************************/
void samd21_GCLK_MAIN_48M_CL(void) {
    NVMCTRL->CTRLB.bit.RWS = NVMCTRL_CTRLB_RWS_HALF_Val;  // flash wait states = 1

   // put Generic Clock Generator 1 as source for Generic Clock Multiplexer 0 (DFLL48M reference)
   GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(0)     | // Generic Clock Multiplexer 0
                       GCLK_CLKCTRL_GEN_GCLK1 | // Generic Clock Generator 1 is source
                       GCLK_CLKCTRL_CLKEN ;
   while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

   // remove the OnDemand mode, Bug http://avr32.icgroup.norway.atmel.com/bugzilla/show_bug.cgi?id=9905
   SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_ENABLE;

   while ((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0);

   SYSCTRL->DFLLMUL.reg = SYSCTRL_DFLLMUL_CSTEP(31)  | // Coarse step is 31, half of the max value
                          SYSCTRL_DFLLMUL_FSTEP(511) | // Fine step is 511, half of the max value
                          SYSCTRL_DFLLMUL_MUL(((48000000ul)/(32768ul))) ; // External 32KHz is the reference

   while ((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0);

   // write full configuration to DFLL control register
   SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_MODE | /* Enable the closed loop mode */
                            SYSCTRL_DFLLCTRL_WAITLOCK |
                            SYSCTRL_DFLLCTRL_QLDIS ; /* Disable Quick lock */

   while ((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0);

   // enable the DFLL
   SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_ENABLE ;

   while (((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKC) == 0) ||
           ((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKF) == 0)); // wait for locks flags

   while ((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0);

   // switch Generic Clock Generator 0 to DFLL48M. CPU will run at 48MHz.
   GCLK->GENDIV.reg = GCLK_GENDIV_ID(0) ; // Generic Clock Generator 0

   while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

   // write Generic Clock Generator 0 configuration
   GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(0)       |
                       GCLK_GENCTRL_SRC_DFLL48M | // Selected source is DFLL 48MHz
                       GCLK_GENCTRL_IDC         | // Set 50/50 duty cycle
                       GCLK_GENCTRL_GENEN ;

   while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);

   _configSystick(48000000);
}

#endif



