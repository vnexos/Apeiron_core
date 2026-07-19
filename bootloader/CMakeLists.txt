# =========================================================
# Copyright (c) 2026 VNExos Inc.
# Bảo lưu mọi quyền.
#
# Được cấp phép theo Giấy phép Độc quyền.
# Xem tệp LICENSE tại thư mục `internal` để biết thêm chi
# tiết.
#
# Quản lý các tệp mã nguồn trong Bộ nạp khởi động chính
# =========================================================

file(GLOB_RECURSE BOOT_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/inc")

foreach(ARCH IN LISTS VNExos_ARCHS)
    set(FUNC_NAME "VNExosBuildEfi_${ARCH}")
    if(COMMAND ${FUNC_NAME})
        if(ARCH STREQUAL "riscv64")
            set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker_grub_riscv64.ld")
            cmake_language(CALL ${FUNC_NAME} "vnexos" "vnexos_db" "root" "${BOOT_SOURCES}" TRUE "${BOOT_LINKER_SCRIPT}")
        else()
            cmake_language(CALL ${FUNC_NAME} "vnexos" "vnexos_db" "root" "vnexos_main" "${BOOT_SOURCES}" TRUE)
        endif()
    endif()
endforeach()
