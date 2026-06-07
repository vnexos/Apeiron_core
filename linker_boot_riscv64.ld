/* =========================================================
 * LINKER SCRIPT CHO BỘ NẠP KHỞI ĐỘNG RISC-V 64-BIT (UEFI)
 *
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép MIT.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * LÝ DO:
 *   UEFI loader load EFI application vào địa chỉ bộ nhớ tùy ý.
 *   RISC-V dùng PC-relative addressing (auipc) nên code tự nhiên
 *   là position-independent — không cần base relocation thực.
 *
 *   Tuy nhiên, PE header yêu cầu không có section tại VAddr=0
 *   (sẽ overlap với PE headers), nên code phải bắt đầu từ 0x1000.
 * =========================================================*/

OUTPUT_FORMAT("elf64-littleriscv")
OUTPUT_ARCH(riscv)

ENTRY(vnexos_main)

SECTIONS {
    /* Code bắt đầu từ 0x1000 — tránh VAddr=0 overlap với PE header */
    . = 0x1000;

    .text : ALIGN(0x1000) {
        *(.text .text.*)
    }

    .rodata : ALIGN(0x1000) {
        *(.rodata .rodata.*)
        *(.data.rel.ro .data.rel.ro.*)
    }

    .data : ALIGN(0x1000) {
        *(.data .data.*)
        *(.sdata .sdata.*)
    }

    .bss : ALIGN(0x1000) {
        *(.bss .bss.*)
        *(.sbss .sbss.*)
    }

    /* Loại bỏ các sections không cần cho UEFI bare-metal */
    /DISCARD/ : {
        *(.dynamic)
        *(.dynsym)
        *(.dynstr)
        *(.gnu.hash)
        *(.hash)
        *(.plt)
        *(.got)
        *(.got.plt)
        *(.comment)
        *(.riscv.attributes)
        *(.eh_frame .eh_frame_hdr)
        *(.note.*)
    }
}
