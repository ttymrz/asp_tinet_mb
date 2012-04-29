/*
 * MicroBlaze Linker Script for aes_s6mb_lx9_g
 * Pre-Built MicroBlaze Hardware Platform : AVS6LX9MBHP311 or AVS6LX9MBHP211
 *
 * $Id: aes_s6mb_lx9_g.ld 12 2012-03-28 12:57:07Z tetsuya $
 */

/* stack_size is defined in target_config.h */
_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x0;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x0;

/* Define Memories in the system */

MEMORY
{
   microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl : ORIGIN = 0x00000050, LENGTH = 0x00007FB0
   MCB3_LPDDR_S0_AXI_BASEADDR : ORIGIN = 0xBC000000, LENGTH = 0x04000000
}

PROVIDE(hardware_init_hook = 0);
PROVIDE(software_init_hook = 0);
PROVIDE(software_term_hook = 0);

/* Specify the default entry point to the program */

STARTUP(start.o)

/* Define the sections, and where they are mapped in memory */

SECTIONS
{
    .vectors.reset : {
        *(.vectors.reset)
    } > MCB3_LPDDR_S0_AXI_BASEADDR

    .vectors.sw_exception : {
        *(.vectors.sw_exception)
    } > MCB3_LPDDR_S0_AXI_BASEADDR

    .vectors.interrupt : {
        *(.vectors.interrupt)
    } > MCB3_LPDDR_S0_AXI_BASEADDR

    .vectors.hw_exception  : {
        *(.vectors.hw_exception)
    } > MCB3_LPDDR_S0_AXI_BASEADDR
    .vectors.reserve : {
        *(.vectors.reserve)
    } > MCB3_LPDDR_S0_AXI_BASEADDR

    . = ALIGN(64);
    .text : {
        *(.text)
        *(.text.*)
        *(.gnu.linkonce.t.*)
    } > MCB3_LPDDR_S0_AXI_BASEADDR

    . = ALIGN(4);
    .rodata : {
        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r.*)
    } > MCB3_LPDDR_S0_AXI_BASEADDR

    /* Alignments by 8 to ensure that _SDA2_BASE_ on a word boundary */
    /* Note that .sdata2 and .sbss2 must be contiguous */
    . = ALIGN(8);
    _ssrw = .;
    .sdata2 : {
        PROVIDE(__sdata2_start = .);
        *(.sdata2)
        *(.sdata2.*)
        *(.gnu.linkonce.s2.*)
        PROVIDE(__sdata2_end = .);
    } > MCB3_LPDDR_S0_AXI_BASEADDR
    . = ALIGN(4);
    .sbss2 : {
        PROVIDE(__sbss2_start = .);
        *(.sbss2)
        *(.sbss2.*)
        *(.gnu.linkonce.sb2.*)
        PROVIDE(__sbss2_end = .);
    } > MCB3_LPDDR_S0_AXI_BASEADDR
    . = ALIGN(8);
    _essrw = .;
    PROVIDE(_SDA2_BASE_ = _ssrw + ((_essrw - _ssrw)/ 2 ));

    . = ALIGN(4);
    .data : {
        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d.*)
    } > MCB3_LPDDR_S0_AXI_BASEADDR

    /* Alignments by 8 to ensure that _SDA_BASE_ on a word boundary */
    /* Note that .sdata and .sbss must be contiguous */
    . = ALIGN(8);
    _ssro = .;
    .sdata : {
        *(.sdata)
        *(.sdata.*)
        *(.gnu.linkonce.s.*)
    } > MCB3_LPDDR_S0_AXI_BASEADDR
    . = ALIGN(4);
    .sbss : {
        PROVIDE(__sbss_start = .);
        *(.sbss)
        *(.sbss.*)
        *(.gnu.linkonce.sb.*)
        PROVIDE(__sbss_end = .);
    } > MCB3_LPDDR_S0_AXI_BASEADDR
    . = ALIGN(8);
    _essro = .;
    PROVIDE(_SDA_BASE_ = _ssro + ((_essro - _ssro)/ 2 ));

    . = ALIGN(4);
    .bss : {
       PROVIDE(__bss_start = .);
       *(.bss)
       *(.bss.*)
       *(.gnu.linkonce.b.*)
       *(COMMON)
       . = ALIGN(4);
       PROVIDE(__bss_end = .);
    } > MCB3_LPDDR_S0_AXI_BASEADDR

    . = ALIGN(4);
    .heap : {
        _heap = .;
        _heap_start = .;
        . += _HEAP_SIZE;
        _heap_end = .;
    } > MCB3_LPDDR_S0_AXI_BASEADDR

    . = ALIGN(4);
    .stack : {
        _stack_end = .;
        . += _STACK_SIZE;
        . = ALIGN(8);
        _stack = .;
        _end = .;
    } > MCB3_LPDDR_S0_AXI_BASEADDR
}

