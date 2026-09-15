/**
 * Copyright (c) 2026 VNExos
 * Bảo lưu mọi quyền.
 *
 * Được cấp phép theo Giấy phép Độc quyền.
 * Xem tệp LICENSE tại thư mục `internal` để biết thêm chi
 * tiết.
 *
 * @file mmaa64.hpp
 * @brief Tệp triển khai các hàm quản lý bộ nhớ của riêng
 * dòng kiến trúc ARM64.
 */
#pragma once

#include <mm.hpp>
#include <stdint.h>

inline uint64_t archMakePte(uint64_t physicalAddress, uint64_t flags, bool nonLeaf)
{
  uint64_t pte = physicalAddress & 0x0000fffffffff000ull;

  if (nonLeaf)
  {
    pte |= 0x3ull; // Table descriptor
  } else
  {
    pte |= (flags & PAGE_HUGE) ? 0x1ull : 0x3ull;

    if (flags & PAGE_USER)
    {
      pte |= (1ull << 6); // AP[1] = 1
    }
    if (!(flags & PAGE_WRITE))
    {
      pte |= (1ull << 7); // AP[2] = 1 (Read-Only)
    }

    if (!(flags & PAGE_EXEC))
    {
      pte |= (1ull << 54) | (1ull << 53); // UXN=1 & PXN=1
    } else if (flags & PAGE_USER)
    {
      pte |= (1ull << 53); // PXN=1 only
    } else
    {
      pte |= (1ull << 54); // UXN=1 only
    }

    if (!(flags & PAGE_GLOBAL))
    {
      pte |= (1ull << 11); // nG = 1
    }

    pte |= (3ull << 8);  // SH = Inner Shareable
    pte |= (1ull << 10); // AF = Access Flag

    if (flags & PAGE_MMIO)
    {
      pte |= (1ull << 2); // AttrIndx = 1 (Device MMIO)
    } else if (flags & PAGE_NOCACHE)
    {
      pte |= (2ull << 2); // AttrIndx = 2 (Normal Non-cacheable)
    }
  }

  return pte;
}

inline uint64_t archToVirtualAddress(uint16_t rootIndex, uint16_t upperIndex, uint16_t middleIndex, uint16_t lowerIndex)
{
  uint64_t va = ((uint64_t)(rootIndex & 0x1FF) << 39) |
                ((uint64_t)(upperIndex & 0x1FF) << 30) |
                ((uint64_t)(middleIndex & 0x1FF) << 21) |
                ((uint64_t)(lowerIndex & 0x1FF) << 12);

  if (rootIndex >= 256)
  {
    va |= 0xFFFF000000000000ULL;
  }

  return va;
}
