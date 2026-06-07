OUTPUT_FORMAT("elf64-littleriscv")
OUTPUT_ARCH(riscv)
ENTRY(kernel_main)

SECTIONS
{
    /* Địa chỉ ảo vùng cao đồng bộ hệ thống */
    . = 0xFFFFFF8080010000;
    _kernel_start = .;

    /* Mã máy */
    .text : ALIGN(0x1000) {
        *(.text .text.*)
        *(.init)
        *(.fini)
    }

    /* Dữ liệu chỉ đọc */
    .rodata : ALIGN(0x1000) {
        *(.rodata .rodata.*)
    }

    /* Dữ liệu có thể ghi (Bao gồm cả Small Data của kiến trúc RISC-V) */
    .data : ALIGN(0x1000) {
        *(.data .data.*)
        *(.sdata .sdata.*)  /* BẮT BUỘC có đối với RISC-V */
    }

    /* Vùng BSS (Bao gồm cả Small BSS) */
    .bss : ALIGN(0x1000) {
        *(.sbss .sbss.*)    /* BẮT BUỘC có đối với RISC-V */
        *(.bss .bss.*)
        *(COMMON)
    }

    . = ALIGN(0x1000);
    _kernel_end = .;
}