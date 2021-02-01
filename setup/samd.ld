/*
*****************************************************************************

      samd.ld

      Universal linker command file for SAMD parts.
      
      The generic memory map is defined below. There are default values
      for the size of each section which can be overwritten with linker
      options.
      
      For example, to override the default value of the stack size include
      the following on the linker command line:
      
      -Xlinker --defsym=__STACK_SIZE=2K
      
      
  
                               FLASH MEMORY MAP
                               
                   ----------------------------------------   __FLASH_SIZE
                  |           EEPROM EMULATION             |
                   ---------------------------------------- 
                  |                                        |
                  ~                                        ~
                  ~             APPLICATION                ~
                  |                                        |
                  |                                        |  __APPLICATION_BASE
                   ----------------------------------------   + __VECTOR_TABLE_SIZE
                  |                                        |
                  |             VECTOR TABLE               |
                  |                                        |
                   ----------------------------------------   __APPLICATION_BASE;
                  |  / / / / / / / / / / / / / / / / / / / |
                  | / / / / / / / / / / / / / / / / / / /  |
                   ----------------------------------------
                  |                                        |
                  |              BOOTLOADER                |
                  |                                        |
                   ----------------------------------------
                  |                                        |
                  |             VECTOR TABLE               |
                  |                                        |
    0x0000_0000   ----------------------------------------    __FLASH_START


                                SRAM MEMORY MAP

                   ----------------------------------------   __SRAM_SIZE
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
    0x20000000    ----------------------------------------   __SRAM_BOTTOM
  
 


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
__VECTOR_TABLE_SIZE   = DEFINED(__VECTOR_TABLE_SIZE)   ? __VECTOR_TABLE_SIZE   : 0x90;

/*
 * If there is a bootloader the application will load above it
 */
 __APPLICATION_BASE   = DEFINED(__APPLICATION_BASE)    ? __APPLICATION_BASE    : 0x0000;

/*
 *          ****** END OVERRIDE USING LINKER COMMAND LINE ******
 */



ENTRY(Reset_Handler)

/* Stack goes at bottom of RAM so an overflow will fault */
__SP_INIT         = __SRAM_BOTTOM + __STACK_SIZE;

__SRAM_BOTTOM     = 0x20000000;
__SRAM_TOP        = __SRAM_BOTTOM + __SRAM_SIZE;

__FLASH_START     = 0x00000000;
__FLASH_END       = __FLASH_START + __FLASH_SIZE;


/* Partition constants */
__LOGFILE_START   = __SRAM_TOP - __LOGFILE_SIZE;
__LOGFILE_END     = __SRAM_TOP;

__HEAP_START      = __LOGFILE_START - __HEAP_SIZE;
__HEAP_END        = __LOGFILE_START;

__STACK_START     = __SRAM_BOTTOM;
__STACK_END       = __SP_INIT;

__TEXT_START      = __APPLICATION_BASE + __VECTOR_TABLE_SIZE;

      
/* Specify the memory areas. m_text and m_ram could be oversized and
 * the section loading commands would allocate storage appropriate
 * to the particular part derivative being used. */
MEMORY
{
    /* Flash */
    m_interrupts     (rx) : ORIGIN = __APPLICATION_BASE,    LENGTH = __VECTOR_TABLE_SIZE
    m_text           (rx) : ORIGIN = __TEXT_START, 	        LENGTH = __FLASH_SIZE - __TEXT_START

    /* SRAM */
    m_ram           (rwx) : ORIGIN = 0x20000000, 		 	LENGTH = __SRAM_SIZE
}

		
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
  
  /* The program code and other initialized data goes into Flash */
  /DISCARD/ :
{
    *(.ARM.exidx*)
    *(.gnu.linkonce.armexidx.*)
}
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */   
     *(.init)          /* standard library functions add these */
     *(.fini)
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
    . = ABSOLUTE(__SRAM_BOTTOM);
    . += __STACK_SIZE;
    . = ALIGN(4);
    __START_BSS = .;
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
  
  /* Verify Flash did not overflow */
  ASSERT(__END_TEXT_AND_DATA <= __FLASH_SIZE, "LINK ERROR: FLASH OVERFLOW")
}
