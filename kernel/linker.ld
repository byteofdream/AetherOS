ENTRY(_start)

SECTIONS
{
    . = 2M;

    .boot : ALIGN(4K) {
        KEEP(*(.multiboot))
        KEEP(*(.boot.text))
        KEEP(*(.boot.data))
    }

    .text : ALIGN(4K) {
        *(.text*)
    }

    .rodata : ALIGN(4K) {
        *(.rodata*)
    }

    .data : ALIGN(4K) {
        *(.data*)
    }

    .bss : ALIGN(4K) {
        *(COMMON)
        *(.bss*)
    }

    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.eh_frame*)
    }
}
