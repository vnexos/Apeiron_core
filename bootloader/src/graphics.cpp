/**
 * Copyright (c) 2026 VNExos Inc.
 * Bảo lưu mọi quyền.
 *
 * Được cấp phép theo Giấy phép Độc quyền.
 * Xem tệp LICENSE tại thư mục `internal` để biết thêm chi
 * tiết.
 *
 * @file graphics.cpp
 * @brief Tệp triển khai của các hàm vẽ đồ họa
 */

#include "graphics.hpp"

// Căn bậc 2 bằng thuật toán Babylon/Newton
static uint32_t sqrt(uint32_t value)
{
  if (value == 0) return 0;
  uint32_t res = value;
  uint32_t pre;
  do
  {
    pre = res;
    res = (res + value / res) / 2;
  } while (pre > res + 1 || res > pre + 1);
  return res;
}

void drawFilledCircle(EFI_GRAPHICS_OUTPUT_BLT_PIXEL* buffer, uint32_t diameter, EFI_GRAPHICS_OUTPUT_BLT_PIXEL* primaryColor,
                      EFI_GRAPHICS_OUTPUT_BLT_PIXEL* secondaryColor)
{
  // Vì bộ đệm là hình vuông khít với hình tròn nên Chiều rộng = Chiều cao = Đường kính
  uint32_t width = diameter;

  // Tính toán tọa độ tâm bằng dấu chấm tĩnh
  int32_t cx = (diameter << 8) / 2;
  int32_t cy = (diameter << 8) / 2;

  // Bán kính thực tế và các vùng biên
  int32_t rOutside = (diameter << 8) / 2;
  int32_t rInside  = rOutside - (1 << 8); // Vùng bắt đầu khử răng cưa

  const int32_t ALPHA_MAX = 256;

  for (uint32_t y = 0; y < diameter; ++y)
  {
    int32_t fixedPointY = (y << 8) + 128;
    int32_t dy          = fixedPointY - cy;
    int32_t dySquare    = (dy * dy) >> 8; // Bình phương xong thì dịch phải để giữ nguyên lề Dấu chấm tĩnh

    for (uint32_t x = 0; x < diameter; ++x)
    {
      int32_t fixedPointX = (x << 8) + 128;
      int32_t dx          = fixedPointX - cx;
      int32_t dxSquare    = (dx * dx) >> 8;

      // Tính khoản cách bình phương từ tâm đến pixel
      uint32_t distSquare = (uint32_t)(dxSquare + dySquare);

      // Khai căn để lấy khoảng cách tuyến tính thực tế
      int32_t dist = sqrt(distSquare << 8);

      int32_t alpha = 0;

      // Định vị Pixel mục tiêu trong bộ đệm hình vuông
      EFI_GRAPHICS_OUTPUT_BLT_PIXEL* pixel = &buffer[y * width + x];

      if (dist <= rInside) // Nằm hoàn toàn bên trong
      {
        // Đậm đặc 100%
        *pixel = *primaryColor;
      } else if (dist >= rOutside) // Nằm hoàn toàn bên ngoài
      {
        // Bỏ qua
        *pixel = *secondaryColor;
      } else // Vùng biên răng cưa
      {
        // Tính toán tỉ lệ mờ nội suy tuyến tính (rOutside - rInside luôn = 256)
        alpha = dist - rInside;
        if (alpha < 0) alpha = 0;
        if (alpha > ALPHA_MAX) alpha = ALPHA_MAX;

        // Trộn số nguyên Dấu chấm động
        pixel->Red      = (uint8_t)((secondaryColor->Red * alpha + primaryColor->Red * (ALPHA_MAX - alpha)) >> 8);
        pixel->Green    = (uint8_t)((secondaryColor->Green * alpha + primaryColor->Green * (ALPHA_MAX - alpha)) >> 8);
        pixel->Blue     = (uint8_t)((secondaryColor->Blue * alpha + primaryColor->Blue * (ALPHA_MAX - alpha)) >> 8);
        pixel->Reserved = 0;
      }
    }
  }
}
