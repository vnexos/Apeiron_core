/**
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép Độc quyền.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 */
OUTPUT_FORMAT(elf64-x86-64)
ENTRY(kernel_main)

SECTIONS
{
    /* Địa chỉ ảo vùng cao dành riêng cho Nhân hệ điều hành */
    . = 0xFFFFFF8080010000;
    _kernel_start = .;

    /* Cần gộp cả các section khởi tạo nếu sử dụng tính năng khởi tạo toàn cục của C++ */
    .text : ALIGN(0x1000) {
        *(.text .text.*)
        *(.init)
        *(.fini)
    }

    .rodata : ALIGN(0x1000) {
        *(.rodata .rodata.*)
    }

    .data : ALIGN(0x1000) {
        *(.data .data.*)
    }

    /* BSS không chiếm dung lượng file, nhưng cần để Kernel biết vùng RAM cần xóa */
    .bss : ALIGN(0x1000) {
        *(.bss .bss.*)
        *(COMMON)
    }

    . = ALIGN(0x1000);
    _kernel_end = .;
}