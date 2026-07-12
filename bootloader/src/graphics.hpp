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
 * @param primaryColor   Màu của hình tròn
 */
void drawFilledCircle(
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* buffer,
    uint32_t                       diameter,
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* primaryColor);

/**
 * Vẽ một hình viên thuốc khít với bộ đệm
 * @param buffer Bộ đệm để vẽ hình viên thuốc
 * @param width  Chiều rộng của hình viên thuốc
 * @param height Chiều cao của hình viên thuốc
 * @param color  Màu của hình viên thuốc
 * @param delta  Chiều rộng của bộ đệm
 */
void drawFilledCapsule(
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* buffer,
    uint32_t                       width,
    uint32_t                       height,
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* color,
    uint32_t                       delta = 0);
