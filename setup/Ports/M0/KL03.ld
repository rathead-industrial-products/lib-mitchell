/*
 * 
 *   KL03.ld
 *   
 */

/* Entry point, needed for the debugger */
ENTRY(Reset_Handler)

/* Stack goes at bottom of RAM so a stack overflow will fault */
__SP_INIT         = __SRAM_L_START + __STACK_SIZE;

/* KL03Z8 Memory Map */
__SRAM_L_SIZE     = 512;
__SRAM_U_SIZE     = 1536;
__SRAM_SIZE       = __SRAM_L_SIZE + __SRAM_U_SIZE;    /* 2K Total */
__FLASH_SIZE      = 8K;
__FLASH_START     = 0x00000000;
__FLASH_END       = __FLASH_START + __FLASH_SIZE;
__SRAM_ANCHOR     = 0x20000000;
__SRAM_L_START    = __SRAM_ANCHOR - __SRAM_L_SIZE;
__SRAM_L_END      = __SRAM_ANCHOR;
__SRAM_U_START    = __SRAM_ANCHOR;
__SRAM_U_END      = __SRAM_ANCHOR + __SRAM_U_SIZE;

/* Partition constants */
__STACK_SIZE      = 1K;
__STACK_START     = __SRAM_L_START;


/* Specify the memory areas */
MEMORY
{
    /* Flash */
    m_interrupts     (rx) : ORIGIN = 0x00000000,        LENGTH = 0x300        /* vector table */
    m_flash_config   (rx) : ORIGIN = 0x00000400,        LENGTH = 0x00000010
    m_text           (rx) : ORIGIN = 0x00000450,        LENGTH = 8K - 450

    /* SRAM */
    m_ram           (rwx) : ORIGIN = 0x20000000 - 512,  LENGTH = 2K
}

/* Define output sections */
SECTIONS
{
  .interrupts :
  {
  __vector_table = .;
    . = ALIGN(4);
    KEEP(*(.vectortable)) /* Startup code */
    . = ALIGN(4);
  } > m_interrupts

  .flash_config :
  {
    . = ALIGN(4);
    KEEP(*(.flashconfig))    /* Flash Configuration Field (FCF) */
    . = ALIGN(4);
  } > m_flash_config

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
    
  /* Stack is at bottom of RAM so that a stack overflow can be trapped */
  .stack :
  {
    . = ALIGN (0x8);
    _sstack = .;
    . += __STACK_SIZE;
    _estack = .;
  } > m_ram
  
  .mtb : /* MTB buffer address as defined by the hardware */
  {
    . = ALIGN(8);
    _mtb_start = .;
    KEEP(*(.mtb_buf))   /* KEEP Micro Trace Buffer - not referenced by application */
    . = ALIGN(8);
    _mtb_end = .;
  } > m_ram
  

  /* Initialized data sections go into RAM, load LMA copy after code */
  .data : AT(___ROM_AT)
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.execute_in_ram) /* code that must run from RAM */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } > m_ram
  
  ___data_size = (_edata - _sdata);
  _etext_and_data = _etext + ___data_size;
  
  /* Uninitialized data section */
  .bss :
  {
     __START_BSS = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    __END_BSS = .;
  } > m_ram


}

