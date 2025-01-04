SECTIONS {
    . = 0x8780000;
    .text : {
        obj/start.o 
        *(.text)
    }
    .rodata ALGIN(4) : {
        *(.rodata)
    }
    .data ALGIN(4) : {
        *(.data)
    }
    __bss_start = .;
    .bss ALIGN(4) : {
        *(.bss*)
        *(COMMON)
    }
    __bss_end = .;
}