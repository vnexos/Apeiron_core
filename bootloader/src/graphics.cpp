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

#include <efilib.hpp>
#include <graphics.hpp>

// Căn bậc 2 bằng thuật toán Babylon/Newton
static uint32_t sqrt_algo(uint32_t value)
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

#define SQRT_CACHE_SIZE 8192
static uint32_t sqrtCacheKeys[SQRT_CACHE_SIZE];
static uint32_t sqrtCacheValues[SQRT_CACHE_SIZE];

static uint32_t cached_sqrt(uint32_t value)
{
  if (value == 0) return 0;
  // Băm giá trị để lấy index
  uint32_t idx = (value ^ (value >> 13) ^ (value >> 21)) & (SQRT_CACHE_SIZE - 1);

  if (sqrtCacheKeys[idx] == value && sqrtCacheValues[idx] != 0)
    return sqrtCacheValues[idx];

  uint32_t res         = sqrt_algo(value);
  sqrtCacheKeys[idx]   = value;
  sqrtCacheValues[idx] = res;
  return res;
}

void drawFilledCircle(EFI_GRAPHICS_OUTPUT_BLT_PIXEL* buffer, uint32_t diameter, EFI_GRAPHICS_OUTPUT_BLT_PIXEL* primaryColor)
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
      int32_t dist = cached_sqrt(distSquare << 8);

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
        continue;
      } else // Vùng biên răng cưa
      {
        // Tính toán tỉ lệ mờ nội suy tuyến tính (rOutside - rInside luôn = 256)
        alpha = dist - rInside;
        if (alpha < 0) alpha = 0;
        if (alpha > ALPHA_MAX) alpha = ALPHA_MAX;

        // Trộn số nguyên Dấu chấm động
        pixel->Red      = (uint8_t)((pixel->Red * alpha + primaryColor->Red * (ALPHA_MAX - alpha)) >> 8);
        pixel->Green    = (uint8_t)((pixel->Green * alpha + primaryColor->Green * (ALPHA_MAX - alpha)) >> 8);
        pixel->Blue     = (uint8_t)((pixel->Blue * alpha + primaryColor->Blue * (ALPHA_MAX - alpha)) >> 8);
        pixel->Reserved = 0;
      }
    }
  }
}

void drawFilledCapsule(EFI_GRAPHICS_OUTPUT_BLT_PIXEL* buffer, uint32_t width, uint32_t height, EFI_GRAPHICS_OUTPUT_BLT_PIXEL* color, uint32_t delta, uint32_t xOffset)
{
  // Chiều rộng tối thiểu phải bằng chiều cao của hình viên thuốc
  if (width < height)
    width = height;
  if (delta == 0)
    delta = width;

  // 2 đầu của hình viên thuốc là hình tròn có đường kính bằng đúng chiều cao của hình viên thuốc
  uint32_t diameter = height;

  // Bán kính thực tế và các vùng biên
  int32_t rOutside = diameter << 7;
  int32_t rInside  = rOutside - (1 << 8);

  // Tọa độ y chung của nửa hình tròn 2 đầu
  int32_t cy = (diameter << 8) / 2;

  // Tọa độ của hình tròn ở đầu
  int32_t cx1 = ((diameter << 8) / 2) + (xOffset << 8);

  // Tọa độ của hình tròn ở đuôi
  int32_t cx2 = ((width + xOffset) << 8) - rOutside;

  const int32_t ALPHA_MAX = 256;

  for (uint32_t y = 0; y < diameter; ++y)
  {
    int32_t  fixedPointY = (y << 8) + 128;
    int32_t  dy          = fixedPointY - cy;
    uint32_t dySquare    = (uint32_t)(dy * dy) >> 8;

    for (uint32_t x = xOffset; x < width + xOffset; ++x)
    {
      EFI_GRAPHICS_OUTPUT_BLT_PIXEL* pixel       = &buffer[y * delta + x];
      int32_t                        fixedPointX = (x << 8) + 128;
      // Hình chữ nhật ở giữa
      if (fixedPointX > cx1 && fixedPointX < cx2)
      {
        *pixel = *color;
      } else
      {
        int32_t  dx       = fixedPointX - (fixedPointX <= cx1 ? cx1 : cx2);
        uint32_t dxSquare = (uint32_t)(dx * dx) >> 8;

        // Khoảng cách bình phương tới tâm
        uint32_t distSquare = dxSquare + dySquare;

        // Khai căn để lấy khoảng cách tuyến tính
        int32_t dist = cached_sqrt(distSquare << 8);

        if (dist <= rInside)
        {
          *pixel = *color;
        } else if (dist >= rOutside)
        {
          continue;
        } else
        {
          // Tính toán tỉ lệ mờ
          int32_t alpha = dist - rInside;
          if (alpha < 0) alpha = 0;
          if (alpha > ALPHA_MAX) alpha = ALPHA_MAX;

          // Trộn số nguyên Dấu chấm động
          pixel->Red      = (uint8_t)((pixel->Red * alpha + color->Red * (ALPHA_MAX - alpha)) >> 8);
          pixel->Green    = (uint8_t)((pixel->Green * alpha + color->Green * (ALPHA_MAX - alpha)) >> 8);
          pixel->Blue     = (uint8_t)((pixel->Blue * alpha + color->Blue * (ALPHA_MAX - alpha)) >> 8);
          pixel->Reserved = 0;
        }
      }
    }
  }
}
