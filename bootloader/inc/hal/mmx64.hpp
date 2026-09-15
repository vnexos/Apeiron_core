/**
 * Copyright (c) 2026 VNExos
 * Bảo lưu mọi quyền.
 *
 * Được cấp phép theo Giấy phép Độc quyền.
 * Xem tệp LICENSE tại thư mục `internal` để biết thêm chi
 * tiết.
 *
 * @file mmx64.hpp
 * @brief Tệp triển khai các hàm quản lý bộ nhớ của riêng
 * dòng kiến trúc x86_64.
 */
#pragma once

#include <mm.hpp>
#include <stdint.h>

inline uint64_t archMakePte(uint64_t physicalAddress, uint64_t flags, bool nonLeaf)
{
  uint64_t pte  = physicalAddress & 0x000ffffffffff000ull;
  pte          |= (1ull << 0); // P

  if (nonLeaf)
  {
    pte |= (1ull << 1); // R/W = 1 (permissive)
    pte |= (1ull << 2); // U/S = 1 (permissive)
  } else
  {
    if (flags & PAGE_WRITE) pte |= (1ull << 1);  // R/W
    if (flags & PAGE_USER) pte |= (1ull << 2);   // U/S
    if (flags & PAGE_GLOBAL) pte |= (1ull << 8); // G
    if (flags & PAGE_HUGE) pte |= (1ull << 7);   // PS

    if (!(flags & PAGE_EXEC))
    {
      pte |= (1ull << 63); // XD / NX
    }

    if (flags & PAGE_MMIO)
    {
      pte |= (1ull << 4) | (1ull << 3); // PCD=1, PWT=1
    } else if (flags & PAGE_NOCACHE)
    {
      pte |= (1ull << 4); // PCD=1
    }
  }

  return pte;
}

inline uint64_t archToVirtualAddress(uint16_t rootIndex, uint16_t upperIndex, uint16_t middleIndex, uint16_t lowerIndex)
{
  // Ghép 4 cấp chỉ mục vào địa chỉ
  uint64_t va = ((uint64_t)(rootIndex & 0x1FF) << 39) |
                ((uint64_t)(upperIndex & 0x1FF) << 30) |
                ((uint64_t)(middleIndex & 0x1FF) << 21) |
                ((uint64_t)(lowerIndex & 0x1FF) << 12);

  // Nếu bit thứ 47 bằng 1 thì đặt các bit trước đó thành 1
  if (va & (1ULL << 47))
  {
    va |= 0xFFFF000000000000ULL;
  }

  return va;
}
