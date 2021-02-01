/*
*****************************************************************************
**
**  MK22FN25612.ld
**
**  MK22FN25612 reference linker command file
**
*****************************************************************************
*/

ENTRY(Reset_Handler)    /* required for the debugger */

/* Stack goes at bottom of RAM so a stack overflow will fault */
__SP_INIT         = __SRAM_L_START + __STACK_SIZE;


/* K22FN256 Memory Map */
__SRAM_L_SIZE     = 16K;
__SRAM_U_SIZE     = 32K;
__SRAM_SIZE       = __SRAM_L_SIZE + __SRAM_U_SIZE;
__FLASH_SIZE      = 256K;
__FLASH_SECTOR    = 2K;

__FLASH_START     = 0x00000000;
__FLASH_END       = __FLASH_START + 256K;

__SRAM_ANCHOR     = 0x20000000;
__SRAM_L_START    = __SRAM_ANCHOR - __SRAM_L_SIZE;
__SRAM_L_END      = __SRAM_ANCHOR;
__SRAM_U_START    = __SRAM_ANCHOR;
__SRAM_U_END      = __SRAM_ANCHOR + __SRAM_U_SIZE;


/* Partition constants */
__USB_DATA_SIZE   = 0x400;    /* bdt and variables */
__USB_DATA_LOC    = __SRAM_U_END - __USB_DATA_SIZE; /* at top of RAM */

__STACK_SIZE      = 4K;
__STACK_START     = __SRAM_L_START;

__VERSION_NUM_OFF = 0x00000410;
__VERSION_NUM_SIZ = 0x20;
__MODULE_NAME_OFF = 0x00000430;
__MODULE_NAME_SIZ = 0x20;

      
/* Specify the memory areas */
MEMORY
{
    /* Flash */
    m_interrupts		 (rx) : ORIGIN = 0x00000000,        LENGTH = 0x300      /* vector table */
    m_gasket_table    (r) : ORIGIN = 0x00000300,        LENGTH = 0x100  	  /* repurpose unused vector table space */
    m_cfmprotrom 		 (rx) : ORIGIN = 0x00000400,        LENGTH = 0x10       /* flash protection control bits */
    m_version_num     (r) : ORIGIN = 0x00000410,        LENGTH = 0x20       /* revision number */
    m_module_name     (r) : ORIGIN = 0x00000430,        LENGTH = 0x20       /* human-readable name */
    m_text           (rx) : ORIGIN = 0x00000450,        LENGTH = 128K       /* total flash is 256K, only using 128K + a little above and below */

    /* SRAM */
    m_ram           (rwx) : ORIGIN = 0x20000000 - 16K,  LENGTH = 64K
}


		
/* Define output sections */
SECTIONS
{
  /* The startup code goes first into Flash */
  .interrupts :
  {
	__vector_table = .;
    . = ALIGN(4);
    KEEP(*(.vectortable)) /* Startup code */
    . = ALIGN(4);
  } > m_interrupts
  
    /* USB gasket jump tables at a fixed location */
  .gasket_table :
  {
    . = ALIGN(4);
    __BL_GASKET = .;
    KEEP(*(.gasket_table))
    . = ALIGN(4);
  } > m_gasket_table
  
  .cfmprotect :
  {
    . = ALIGN(4);
  KEEP(*(.cfmconfig)) /* Flash Configuration Field (FCF) */
  . = ALIGN(4);
  } > m_cfmprotrom
  
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

  /* The program code and other data goes into Flash */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */    
	. = ALIGN(4);
    _etext = .;        /* define a global symbol at end of code */
    ___ROM_AT = .;
  } > m_text
  
  /* Uninitialized data section */
  .bss :
  {
    . += __SRAM_SIZE - __USB_DATA_SIZE;
    . = ALIGN(512);
    __START_BSS = .;
    *ee_usb_driver_kinetis.o(.bss.ee_usb_drvr_bdt) /* BDT must go first on 512K byte boundary */
    *ee_usb_driver_kinetis.o(.bss*)
    *ee_usb.o.o(.bss*)
    *ee_usb_msc.o.o(.bss*)
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __END_BSS = .;
  } > m_ram

  /* Initialized data sections go into RAM, load LMA copy after code */
  .data : AT(___ROM_AT)
  {
    . = ALIGN(4);
    _sdata = .;               /* create a global symbol at data start */
    KEEP(*(.execute_in_ram))  /* code that must run from RAM */
    *ee_usb*(.data)           /* .data sections from usb stack */
    *(.data)                  /* .data sections */
    *(.data*)                 /* .data* sections */
    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } > m_ram
  ___data_size = (_edata - _sdata);
  _etext_and_data = _etext + ___data_size;

}
