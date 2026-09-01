/**
 * Copyright (c) 2026 VNExos
 * Bảo lưu mọi quyền.
 *
 * Được cấp phép theo Giấy phép Độc quyền.
 * Xem tệp LICENSE tại thư mục `internal` để biết thêm chi
 * tiết.
 */
SECTIONS
{
    /* Đặt địa chỉ gốc ảo là 0 để tiện tính toán */
    . = 0x0;
    _image_start = .;

    /* KHỐI CODE: Làm tròn theo trang 4KB */
    .text : ALIGN(0x1000) {
        _text_start = .;

        KEEP(*(.text._Z5entryv));      /* Ép hàm mồi tại vị trí 0x0 */
        *(.text .text.*)
        
        KEEP(*(.init))             /* Mã máy khởi tạo hệ thống cổ điển */
        KEEP(*(.fini))             /* Mã máy hủy hệ thống cổ điển */

        _text_end = .;
    }

    /* KHỐI DỮ LIỆU CHUNG: Làm tròn theo trang 4KB */
    .rodata : ALIGN(0x1000) {
        _rodata_start = .;
        *(.rodata .rodata.*)
        _rodata_end = .;
    }

    .data : ALIGN(0x1000) {
        _data_start = .;
        *(.data .data.*)

        ALIGN(8);
        _init_array_start = .;
        KEEP(*(.init_array))
        KEEP(*(.init_array.*))
        _init_array_end = .;

        ALIGN(8);
        _fini_array_start = .;
        KEEP(*(.fini_array))
        KEEP(*(.fini_array.*))
        _fini_array_end = .;

        ALIGN(8);
        KEEP(*(.vnexos_usx_import.ptr))
        
        _data_end = .;
    }

    .vnexos_usx_import : ALIGN(0x1000) {
        _usx_import_start = .;
        *(.vnexos_usx_import.str)
        *(.vnexos_usx_import.desc)
        *(.vnexos_usx_import.sym)
        _usx_import_end = .;
    }

    /* KHỐI DỮ LIỆU ẢO: Làm tròn theo trang 4KB */
    .bss : ALIGN(0x1000) {
        _bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        _bss_end = .;
    }

    .vnexos_usx_import : ALIGN(0x1000) {
        _usx_import_start = .;
        *(.vnexos_usx_import.str)
        *(.vnexos_usx_import.desc)
        *(.vnexos_usx_import.sym)
        _usx_import_end = .;
    }

    /DISCARD/ : { 
        *(.comment)
        *(.note*)
        *(.eh_frame*)

        /* QUÉT SẠCH RÁC DYNAMIC CỦA -fPIE: */
        *(.rela*)
        *(.interp)
        *(.dynamic)
        *(.dynsym)
        *(.dynstr)
        *(.gnu.hash)
        *(.hash)
        *(.got)
        *(.plt)
    }
}