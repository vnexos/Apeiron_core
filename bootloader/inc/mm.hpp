/**
 * Copyright (c) 2026 VNExos
 * Bảo lưu mọi quyền.
 *
 * Được cấp phép theo Giấy phép Độc quyền.
 * Xem tệp LICENSE tại thư mục `internal` để biết thêm chi
 * tiết.
 *
 * @file mm.hpp
 * @brief Tệp định nghĩa các hàm liên quan tới việc thao
 * tác trên bộ nhớ.
 */
#pragma once

#include <efi.hpp>

bool initMemoryManagement(EFI_BOOT_SERVICES* BootServices);
