/*
 *  @(#) $Id: mbelf.ld 5 2012-01-14 09:35:55Z tetsuya $ 
 */

PROVIDE(hardware_init_hook = 0);
PROVIDE(software_init_hook = 0);
PROVIDE(software_term_hook = 0);
/*
 * Specify the default entry point to the program
 */
STARTUP(start.o)

SECTIONS
{
    .vectors.reset 0x00000000 : {
       *(.vectors.reset)
    } 

    .vectors.sw_exception 0x00000008 : {
       *(.vectors.sw_exception)
    } 

    .vectors.interrupt 0x00000010 : {
       *(.vectors.interrupt)
    } 

    .vectors.hw_exception 0x00000020 : {
       *(.vectors.hw_exception)
    }

    .vectors.reserve 0x00000028 : {
       *(.vectors.reserve)
    }

    .text : {
       __text = . ;
       *(.text)
       *(.text.*)
       *(.gnu.linkonce.t.*)
    } 
    _extext = . ;

    . = ALIGN(4);
    .rodata : {
       __rodata_start = .;
       *(.rodata)
       *(.rodata.*)
       *(.gnu.linkonce.r.*)
       __rodata_end = .;
    } 

    . = ALIGN(4);
    __idata_start = .;
    .data : AT(__idata_start)
    {
       __data_start = .;
       *(.data)
       *(.data.*)
       *(.gnu.linkonce.d.*)
    } 
    __idata_end = __idata_start + SIZEOF(.data);

   /*
    * Alignments by 8 to ensure that _SDA2_BASE_ on a word boundary
    */
    . = ALIGN(8);
    _ssrw = .;
    __isdata2_start = __idata_start + (_ssrw - __data_start);
    .sdata2 :AT(__isdata2_start)
    {
        __sdata2_start = .;
        *(.sdata2)
        *(.sdata2.*)
        *(.gnu.linkonce.s2.*)
    } 
    . = ALIGN(8);
    _essrw = .;
    __isdata2_end = __isdata2_start + SIZEOF(.sdata2);
    _ssrw_size = _essrw - _ssrw;
    _SDA2_BASE_ = _ssrw + (_ssrw_size / 2 );


   /*
    * Alignments by 8 to ensure that _SDA_BASE_ on a word boundary 
    * Note that .sdata and .sbss must be contiguous
    */
    . = ALIGN(8);
     _ssro = .;
    __isdata_start = __idata_start + (_ssro - __data_start);
    .sdata : AT(__isdata_start)
    {
        __sdata_start = .;
        *(.sdata)
        *(.sdata.*)
        *(.gnu.linkonce.s.*)
    } 
    __isdata_end = __isdata_start + SIZEOF(.sdata);

    . = ALIGN(4);
    __bss_start = . ;
    .sbss : {
      *(.sbss)
      *(.sbss.*)
      *(.gnu.linkonce.sb.*)
    } 

    . = ALIGN(8);
    _essro = .;
    _ssro_size = _essro - _ssro;
    _SDA_BASE_ = _ssro + (_ssro_size / 2 );

    /*
     * Define space for the stack and heap 
     * Note that variables _heap must be set to the beginning of this area 
     * and _stack set to the end of this area
     */
   . = ALIGN(4);
   _heap = .;
   .bss : {
    *(.bss)
    *(COMMON)
   } 
   _stack = .;
   . = ALIGN(4);
   _end = .;
    .comment 0 : { *(.comment) }	
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }	
}

