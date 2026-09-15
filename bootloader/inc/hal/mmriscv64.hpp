/**
 * Copyright (c) 2026 VNExos
 * Bảo lưu mọi quyền.
 *
 * Được cấp phép theo Giấy phép Độc quyền.
 * Xem tệp LICENSE tại thư mục `internal` để biết thêm chi
 * tiết.
 *
 * @file mmriscv64.hpp
 * @brief Tệp triển khai các hàm quản lý bộ nhớ của riêng
 * dòng kiến trúc RISC-V 64.
 */
#pragma once

#include <mm.hpp>
#include <stdint.h>

inline uint64_t archMakePte(uint64_t physicalAddress, uint64_t flags, bool nonLeaf)
{
  uint64_t pte  = ((physicalAddress >> 12) & 0xFFFFFFFFFFFull) << 10;
  pte          |= (1ull << 0); // V

  if (!nonLeaf)
  {
    if (flags & PAGE_WRITE)
    {
      pte |= (1ull << 2) | (1ull << 1); // W=1 kéo theo R=1
    } else if (flags & PAGE_READ)
    {
      pte |= (1ull << 1); // R only
    }

    if (flags & PAGE_EXEC) pte |= (1ull << 3);   // X
    if (flags & PAGE_USER) pte |= (1ull << 4);   // U
    if (flags & PAGE_GLOBAL) pte |= (1ull << 5); // G

    pte |= (1ull << 6);                          // A
    if (flags & PAGE_WRITE)
    {
      pte |= (1ull << 7); // D
    }

    if (flags & PAGE_MMIO)
    {
      pte |= (2ull << 61); // PBMT = IO
    } else if (flags & PAGE_NOCACHE)
    {
      pte |= (1ull << 61); // PBMT = NC
    }
  }

  return pte;
}

inline uint64_t archToVirtualAddress(uint16_t rootIndex, uint16_t upperIndex, uint16_t middleIndex, uint16_t lowerIndex)
{
  // Sv48: VPN[3] -> root, VPN[2] -> upper, VPN[1] -> middle, VPN[0] -> lower
  uint64_t va = ((uint64_t)(rootIndex & 0x1FF) << 39) |
                ((uint64_t)(upperIndex & 0x1FF) << 30) |
                ((uint64_t)(middleIndex & 0x1FF) << 21) |
                ((uint64_t)(lowerIndex & 0x1FF) << 12);

  // Chuẩn RISC-V yêu cầu sign-extend từ bit 47
  if (va & (1ULL << 47))
  {
    va |= 0xFFFF000000000000ULL;
  }

  return va;
}
