# =========================================================
# Copyright (c) 2026 VNExos
# Bảo lưu mọi quyền.
#
# Được cấp phép theo Giấy phép Độc quyền.
# Xem tệp LICENSE tại thư mục `internal` để biết thêm chi
# tiết.
#
# Quản lý toàn bộ nhân lõi nội bộ của Hệ điều hành
# =========================================================

set(BOOT_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker_boot_riscv64.ld")
set(KERN_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker_kern.ld")

add_subdirectory(bootloader)
