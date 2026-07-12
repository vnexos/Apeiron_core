/**
 * Copyright (c) 2026 VNExos Inc.
 * Bảo lưu mọi quyền.
 *
 * Được cấp phép theo Giấy phép Độc quyền.
 * Xem tệp LICENSE tại thư mục `internal` để biết thêm chi
 * tiết.
 *
 * @file graphics.hpp
 * @brief Tệp định nghĩa của các hàm vẽ đồ họa
 */
#pragma once

#include <efi.hpp>

/**
 * Vẽ một hình tròn khít với bộ đệm
 * @param buffer         Bộ đệm để vẽ hình tròn lên, phải là hình
 *                       vuông có cạnh bằng đúng đường kính của
 *                       hình tròn
 * @param diameter       Đường kính của hình tròn
 * @param primaryColor   Màu gốc của hình tròn
 * @param secondaryColor Màu nền sau hình tròn
 */
void drawFilledCircle(
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* buffer,
    uint32_t                       diameter,
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* primaryColor,
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* secondaryColor);
