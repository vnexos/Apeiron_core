# =========================================================
# Copyright (c) 2026 VNExos
# Bảo lưu mọi quyền.
#
# Được cấp phép theo Giấy phép Độc quyền.
# Xem tệp LICENSE tại thư mục `internal` để biết thêm chi
# tiết.
#
# Quản lý các tệp mã nguồn Trong nhân lõi
# =========================================================

set(BIN_LIST "")

file(GLOB_RECURSE KERNEL_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.asm
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/inc")

foreach(ARCH IN LISTS VNExos_ARCHS)
    set(FUNC_NAME "VNExosBuildBinMap_${ARCH}")
    if(COMMAND ${FUNC_NAME})
        cmake_language(CALL ${FUNC_NAME} "kernel" "root" "${KERN_LINKER_SCRIPT}" "${KERNEL_SOURCES}")
        set_property(GLOBAL APPEND PROPERTY VNExos_ALL_TARGET ${TARGET_NAME})

        list(APPEND BIN_LIST ${RESULT_FILE})
    endif()
endforeach()

message("---- ${BIN_LIST}")
