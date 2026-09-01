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

set(FINAL_BIN "${VNExos_SYSROOT_KERN_DIR}/apeiron.kern")
set(RESULT_LIST "")
set(FINAL_TARGET "VNExosKernelAll")

file(GLOB_RECURSE KERNEL_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.asm
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/inc")

set(BIN_FILE_TARGET "")

foreach(ARCH IN LISTS VNExos_ARCHS)
    set(FUNC_NAME "VNExosBuildBinMap_${ARCH}")
    if(COMMAND ${FUNC_NAME})
        cmake_language(CALL ${FUNC_NAME} "kernel" "root" "${KERN_LINKER_SCRIPT}" "${KERNEL_SOURCES}")
        list(APPEND BIN_FILE_TARGET ${TARGET_NAME})

        list(APPEND RESULT_LIST ${RESULT_FILE})
    endif()
endforeach()

# Thêm đuôi tệp vào danh sách
list(TRANSFORM RESULT_LIST APPEND ".bin" OUTPUT_VARIABLE BIN_LIST)
list(TRANSFORM RESULT_LIST APPEND ".map" OUTPUT_VARIABLE MAP_LIST)

add_custom_command(
    OUTPUT ${FINAL_BIN}
    DEPENDS ${BIN_FILE_TARGET}
    COMMAND ${VNExos_TOOL_PACKUSX} 
        --bin ${BIN_LIST} 
        --map ${MAP_LIST} 
        --arch ${VNExos_ARCHS}
        --out ${FINAL_BIN}
        --version 0.1-alpha1
    COMMENT "[ VNExos ] Đã đóng gói Nhân lõi thành tệp: ${FINAL_BIN}"
    VERBATIM
)

add_custom_target(${FINAL_TARGET} ALL DEPENDS ${FINAL_BIN})

set_property(GLOBAL APPEND PROPERTY VNExos_ALL_TARGET ${FINAL_TARGET})
