/* =========================================================
 * TẬP LỆNH LIÊN KẾT CHO BỘ NẠP KHỞI ĐỘNG RISC-V 64-BIT (UEFI)
 *
 * Copyright (c) 2026 VNExos Inc.
 *
 * Được cấp phép theo Giấy phép Độc quyền.
 * Xem tệp LICENSE tại thư mục gốc để biết thêm chi tiết.
 *
 * LÝ DO:
 *   UEFI loader nạp chương trình EFI vào địa chỉ bộ nhớ tùy ý.
 *   RISC-V dùng địa chỉ hóa tương đối theo bộ đếm chương trình (auipc)
 *   nên mã tự nhiên độc lập vị trí — không cần tái địa chỉ hóa thực sự.
 *
 *   Tuy nhiên, tiêu đề PE yêu cầu không có section tại Địa chỉ ảo=0
 *   (sẽ chồng lấp với tiêu đề PE), nên mã phải bắt đầu từ 0x1000.
 * =========================================================*/

OUTPUT_FORMAT("elf64-littleriscv")
OUTPUT_ARCH(riscv)

ENTRY(vnexos_main)

SECTIONS {
    /* Mã lệnh bắt đầu từ 0x1000 — tránh Địa chỉ ảo=0 chồng lấp với tiêu đề PE */
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

    /* Loại bỏ các section không cần thiết trong môi trường bare-metal UEFI */
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
