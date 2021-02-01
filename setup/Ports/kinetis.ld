/*
*****************************************************************************

      kinetis.ld

      Universal linker command file for Kinetis parts.
      
      The generic memory map is defined below. There are default values
      for the size of each section which can be overwritten with linker
      options.
      
      For example, to override the default value of the stack size include
      the following on the linker command line:
      
      -Xlinker --defsym=__STACK_SIZE=2K
      
      
      
      
  
  
                               FLASH MEMORY MAP
                                                            
                   ----------------------------------------   __FLASH_SIZE
                  |                                        |
                  |    BOOTLOADER IMAGE WHILE UPDATING     |
                  |                                        |
                   ----------------------------------------   __FLASH_SIZE -  32K
                  |                                        |
                  ~                                        ~
                  ~             APPLICATION                ~
                  |                                        |
                  |                                        |
    0x0000_8450    ----------------------------------------
                  |         REVISION / DESCRIPTION         |
    0x0000_8410    ----------------------------------------
                  |                                        |
                  |             VECTOR TABLE               |
                  |                                        |
    0x0000_8000    ----------------------------------------
                  |  / / / / / / / / / / / / / / / / / / / |
                  | / / / / / / / / / / / / / / / / / / /  |
                   ----------------------------------------
                  |                                        |
                  |              BOOTLOADER                |
                  |                                        |
    0x0000_0450    ----------------------------------------
                  |         REVISION / DESCRIPTION         |
    0x0000_0410    ----------------------------------------
                  |              CFMPROTROM                |
    0x0000_0400    ----------------------------------------
                  |           BOOTLOADER GASKET            |
    0x0000_0300    ----------------------------------------
                  |                                        |
                  |             VECTOR TABLE               |
                  |                                        |
    0x0000_0000   ----------------------------------------


                                SRAM MEMORY MAP

                   ----------------------------------------   __SRAM_SIZE
                  |                                        |
                  |             USB VARIABLES              |
                  |                                        |
                   ----------------------------------------   __USB_DATA_LOC
                  |             RELOCATED CODE             |
                   ----------------------------------------   __RELOCATED_CODE_START
                  |               LOGFILE                  |
                   ----------------------------------------   __LOGFILE_START
                  |                                        |
                  |                 HEAP                   |
                  |                                        |
                   ----------------------------------------   __HEAP_START
                  |                                        |
                  ~                                        ~
                  ~                                        ~
                  |                .mtb                    |
                  |                .data                   |
                  |                .bss                    |
                  |                                        |
                   ----------------------------------------   __SP_INIT
                  |                                        |
                  |                 STACK                  |
                  |                                        |
                   ----------------------------------------   SRAM_BOTTOM
**  
**  
**  
**
*****************************************************************************
*/


/*
 *            ****** OVERRIDE USING LINKER COMMAND LINE ******
 */

/* 
 * Each part type and derivative can have different Flash/SRAM sizes.
 * Default values are assigned here.
 */
__SRAM_SIZE     = DEFINED(__SRAM_SIZE)    ? __SRAM_SIZE    : 4K;
__FLASH_SIZE    = DEFINED(__FLASH_SIZE)   ? __FLASH_SIZE   : 16K;
__FLASH_SECTOR  = DEFINED(__FLASH_SECTOR) ? __FLASH_SECTOR : 2K;

/*
 * Memory map sizing options. Default values are assigned here.
 */
__STACK_SIZE          = DEFINED(__STACK_SIZE)          ? __STACK_SIZE          : 2K;
__HEAP_SIZE           = DEFINED(__HEAP_SIZE)           ? __HEAP_SIZE           : 0K;
__LOGFILE_SIZE        = DEFINED(__LOGFILE_SIZE)        ? __LOGFILE_SIZE        : 0K;
__RELOCATED_CODE_SIZE = DEFINED(__RELOCATED_CODE_SIZE) ? __RELOCATED_CODE_SIZE : 0K;
__USB_DATA_SIZE       = DEFINED(__USB_DATA_SIZE)       ? __USB_DATA_SIZE       : 0K;

/*
 *          ****** END OVERRIDE USING LINKER COMMAND LINE ******
 */



ENTRY(Reset_Handler)    /* required for the debugger */

/* Stack goes at bottom of RAM so an overflow will fault */
__SP_INIT         = __SRAM_BOTTOM + __STACK_SIZE;

__SRAM_L_SIZE     = __SRAM_SIZE / 4;
__SRAM_U_SIZE     = 3 * __SRAM_L_SIZE;

__SRAM_ANCHOR     = 0x20000000;
__SRAM_BOTTOM     = __SRAM_ANCHOR - __SRAM_L_SIZE;
__SRAM_TOP        = __SRAM_ANCHOR + __SRAM_U_SIZE;

__FLASH_START     = 0x00000000;
__FLASH_END       = __FLASH_START + __FLASH_SIZE;


/* Partition constants */
/* __USB_DATA_SIZE   = 0x400;                        bdt and variables for USB parts */
__USB_DATA_LOC    = __SRAM_TOP - __USB_DATA_SIZE; /* must be 512 byte aligned */

__RELOCATED_CODE_START  = __USB_DATA_LOC - __RELOCATED_CODE_SIZE;
__RELOCATED_CODE_END    = __USB_DATA_LOC;

__LOGFILE_START   = __RELOCATED_CODE_START - __LOGFILE_SIZE;
__LOGFILE_END     = __RELOCATED_CODE_START;

__HEAP_START      = __LOGFILE_START - __HEAP_SIZE;
__HEAP_END        = __LOGFILE_START;

__STACK_START     = __SRAM_BOTTOM;
__STACK_END       = __SP_INIT;

__VERSION_NUM_OFF = 0x00000410;
__VERSION_NUM_SIZ = 0x20;
__MODULE_NAME_OFF = 0x00000430;
__MODULE_NAME_SIZ = 0x20;

      
/* Specify the memory areas. m_text and m_ram are oversized. The section
 * loading commands will allocate storage appropriate to the particular
 * part derivative being used. */
MEMORY
{
    /* Flash */
    m_interrupts		 (rx) : ORIGIN = 0x00000000,        LENGTH = 0x300      /* vector table */
    m_gasket_table    (r) : ORIGIN = 0x00000300,        LENGTH = 0x100  	  /* repurpose unused vector table space */
    m_cfmprotrom 		 (rx) : ORIGIN = 0x00000400,        LENGTH = 0x10       /* flash protection control bits */
    m_version_num     (r) : ORIGIN = 0x00000410,        LENGTH = 0x20       /* revision number */
    m_module_name     (r) : ORIGIN = 0x00000430,        LENGTH = 0x20       /* human-readable name */
    m_text           (rx) : ORIGIN = 0x00000450,        LENGTH = 1M - 0x450

    /* SRAM */
    m_ram           (rwx) : ORIGIN = 0x20000000 - 64K,  LENGTH = 256K
}
__M_RAM_START     = 0x20000000 - 64K;

		
/* Define output sections */
SECTIONS
{
  /*
   *    *** START FLASH CONFIGURATION ***
   */
    
  /* Vector table */
  .interrupts :
  {
	__vector_table = .;
    . = ALIGN(4);
    KEEP(*(.vectortable)) /* Startup code */
    . = ALIGN(4);
  } > m_interrupts
  
  
    /* USB gasket jump table */
  .gasket_table :
  {
    . = ALIGN(4);
    __BL_GASKET = .;
    KEEP(*(.gasket_table))
    . = ALIGN(4);
  } > m_gasket_table
  
  
  /* Flash Configuration Field (FCF) */
  .cfmprotect :
  {
    . = ALIGN(4);
  KEEP(*(.cfmconfig))
  . = ALIGN(4);
  } > m_cfmprotrom
  
  
  /* Version number and module name */
  .version_num :
  {
    . = ALIGN(4);
  KEEP(*(.version_num))
  . = ALIGN(4);
  } > m_version_num
  
  
  .module_name :
  {
    . = ALIGN(4);
  KEEP(*(.module_name))
  . = ALIGN(4);
  } > m_module_name

  
  /* The program code and other initialized data goes into Flash */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */    
	. = ALIGN(4);
   __END_TEXT = .;    /* define a global symbol at end of code */
  } > m_text
  
  
  /*
   *    *** END FLASH CONFIGURATION ***
   *    
   *    *** START SRAM CONFIGURATION ***
   */  
  
  /* Uninitialized (zero-initialized) data section after stack */
  .bss :
  {
      . += (__SRAM_BOTTOM -__M_RAM_START);
      . += __STACK_SIZE;
      . = ALIGN(4);
    __START_BSS = .;
    *ee_usb_driver_kinetis.o(.bss*)
    *ee_usb.o.o(.bss*)
    *ee_usb_msc.o.o(.bss*)
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __END_BSS = .;
  } > m_ram

  
  /* Initialized data sections, load LMA copy after code */
  .data : AT(__END_TEXT)
  {
    . = ALIGN(4);
    __START_DATA = .;         /* create a global symbol at data start */
    *ee_usb*(.data)           /* .data sections from usb stack */
    *(.data)                  /* .data sections */
    *(.data*)                 /* .data* sections */
    . = ALIGN(4);
    __END_DATA = .;           /* define a global symbol at data end */
  } > m_ram
  __DATA_SIZE = (__END_DATA - __START_DATA);
  __END_TEXT_AND_DATA = __END_TEXT + __DATA_SIZE;
  
  
  /* MTB buffer */
  .mtb :
  {
    . = ALIGN(8);
    __START_MTB = .;
    KEEP(*(.mtb_buf))   /* KEEP Micro Trace Buffer - not referenced by application */
    . = ALIGN(8);
    __END_MTB = .;
  } > m_ram
  
  /* Verify all SRAM contents fit into allocated area */
  ASSERT(__END_MTB <= __HEAP_START, "LINK ERROR: SRAM OVERFLOW")

  . = __HEAP_START;
  __START_HEAP = .;
  . += __HEAP_SIZE;
  __END_HEAP = .;
  
  . = __LOGFILE_START;
  __START_LOGFILE = .;
  . += __LOGFILE_SIZE;
  __END_LOGFILE = .;
  

  /* RAM resident code, load LMA copy after code and text */
  . = __RELOCATED_CODE_START;
  .data_eir : AT(__END_TEXT_AND_DATA)
  {
    . = ALIGN(4);
    __START_EIR = .;          /* create a global symbol at ram code start */
    KEEP(*(.execute_in_ram))  /* code that must run from RAM */
    . = ALIGN(4);
    __END_EIR = .;            /* define a global symbol at code end */
  } /* > m_ram */             /* goes in m_ram but load address is specified above the section declarator */
  __EIR_SIZE = (__END_EIR - __START_EIR);
  __END_TEXT_AND_DATA_AND_EIR = __END_TEXT_AND_DATA + __EIR_SIZE;
  
  
  /* Verify Flash did not overflow */
  ASSERT(__END_TEXT_AND_DATA_AND_EIR <= __FLASH_SIZE, "LINK ERROR: FLASH OVERFLOW")
  
  /* Verify RAM resident code fits in allocated section */
  ASSERT(__END_EIR <= __USB_DATA_LOC, "LINK ERROR: RELOCATED CODE SECTION OVERFLOW")
  
  
  /* USB buffer descriptor table into top of SRAM */
  . = __USB_DATA_LOC;
  .bss_usb :
  {
    . = ALIGN(512);
    __START_USB_BDT = .;
    *ee_usb_driver_kinetis.o(.bss.ee_usb_drvr_bdt)
    __END_USB_BDT = .;
  } /* > m_ram */             /* goes in m_ram but load address is specified above the section declarator */


}
