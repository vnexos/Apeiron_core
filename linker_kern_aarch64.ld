/**
 * Copyright (c) 2026 VNExos Inc.
 * Bảo lưu mọi quyền.
 *
 * Được cấp phép theo Giấy phép Độc quyền.
 * Xem tệp LICENSE tại thư mục `internal` để biết thêm chi
 * tiết.
 */
OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(kernel_main)

SECTIONS
{
    /* Địa chỉ ảo vùng cao dành cho Kernel ARM64 */
    . = 0xFFFFFF8080010000;
    _kernel_start = .;

    /* Mã máy - Đảm bảo căn chỉnh 4KB (0x1000) để cấu hình trang bộ nhớ (Translation Tables) */
    .text : ALIGN(0x1000) {
        *(.text .text.*)
        *(.init)
        *(.fini)
    }

    /* Dữ liệu chỉ đọc */
    .rodata : ALIGN(0x1000) {
        *(.rodata .rodata.*)
    }

    /* Dữ liệu có thể ghi */
    .data : ALIGN(0x1000) {
        *(.data .data.*)
    }

    /* Vùng BSS - Cần căn chỉnh chặt chẽ để Kernel xóa RAM bằng các lệnh tối ưu (như STP) */
    .bss : ALIGN(0x1000) {
        *(.bss .bss.*)
        *(COMMON)
    }

    . = ALIGN(0x1000);
    _kernel_end = .;
}