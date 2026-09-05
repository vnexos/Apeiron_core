/**
 * Copyright (c) 2026 VNExos
 * Bảo lưu mọi quyền.
 *
 * Được cấp phép theo Giấy phép Độc quyền.
 * Xem tệp LICENSE tại thư mục `internal` để biết thêm chi
 * tiết.
 *
 * @file mm.cpp
 * @brief Tệp triển khai các hàm liên quan tới việc thao
 * tác trên bộ nhớ.
 */
#include <efilib.hpp>
#include <mm.hpp>
#include <stdint.h>
#include <string.hpp>

using namespace EFI;

/* TRANG ĐẦU VÀ TRANG CUỐI CỦA VÙNG NHỚ LỚN NHẤT */
uint64_t startMaxRegion    = 0;
uint64_t ptrMaxRegion      = 0;
uint64_t endMaxRegion      = 0;
uint64_t start2ndMaxRegion = 0;
uint64_t end2ndMaxRegion   = 0;

bool initMemoryManagement(EFI_BOOT_SERVICES* BootServices, EFI_MEMORY_DESCRIPTOR** _map, uint64_t* _mapSize, uint64_t* _descriptorSize)
{
  EFI_STATUS             status;
  EFI_MEMORY_DESCRIPTOR* map     = nullptr;
  uint64_t               mapSize = 0;
  uint64_t               mapKey;
  uint64_t               descriptorSize;
  uint32_t               descriptorVersion;

  status   = BootServices->GetMemoryMap(&mapSize, map, &mapKey, &descriptorSize, &descriptorVersion);
  mapSize += 4 * descriptorSize;
  status   = BootServices->AllocatePool(EfiLoaderData, mapSize, (void**)&map);

  if (EFI_ERROR(status))
  {
    printf("LOI [7]: Co loi xay ra trong qua trinh cap phat bo nho cho ban do.");
    waitForKey();
    printf("\n");
    return false;
  }

  status = BootServices->GetMemoryMap(&mapSize, map, &mapKey, &descriptorSize, &descriptorVersion);

  if (EFI_ERROR(status))
  {
    printf("LOI [8]: Co loi xay ra trong qua trinh lay ban do bo nho. (Code %x)", status);
    waitForKey();
    printf("\n");
    return false;
  }

  *_map            = map;
  *_mapSize        = mapSize;
  *_descriptorSize = descriptorSize;

  uint64_t numEntries    = mapSize / descriptorSize;
  uint8_t* maxDesc       = (uint8_t*)map;
  uint8_t* secondMaxDesc = (uint8_t*)map;
  uint8_t* ptr           = maxDesc + descriptorSize;

  for (uint64_t i = 1; i < numEntries; ++i)
  {
    EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)ptr;
    if (desc->Type != EfiConventionalMemory)
    {
      ptr += descriptorSize;
      continue;
    }

    endMaxRegion = desc->PhysicalStart + (desc->NumberOfPages * 0x1000ull);
    printf("- 0x%x - 0x%x (%d)\n", desc->PhysicalStart, endMaxRegion, desc->NumberOfPages);

    if (((EFI_MEMORY_DESCRIPTOR*)maxDesc)->NumberOfPages < desc->NumberOfPages)
      secondMaxDesc = maxDesc, maxDesc = ptr;
    else if (((EFI_MEMORY_DESCRIPTOR*)secondMaxDesc)->NumberOfPages < desc->NumberOfPages)
      secondMaxDesc = ptr;

    ptr += descriptorSize;
  }

  EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)maxDesc;
  endMaxRegion                = desc->PhysicalStart + (desc->NumberOfPages * 0x1000ull);
  printf("Vung nho lon nhat: 0x%x - 0x%x (%d)\n", desc->PhysicalStart, endMaxRegion, desc->NumberOfPages);
  startMaxRegion = desc->PhysicalStart;

  desc            = (EFI_MEMORY_DESCRIPTOR*)secondMaxDesc;
  end2ndMaxRegion = desc->PhysicalStart + (desc->NumberOfPages * 0x1000ull);
  printf("Vung nho lon nhi: 0x%x - 0x%x (%d)\n", desc->PhysicalStart, end2ndMaxRegion, desc->NumberOfPages);
  start2ndMaxRegion = desc->PhysicalStart;

  return true;
}

uint64_t makePte(uint64_t physicalAddress, uint64_t flags)
{
  uint64_t   pte     = 0;
  const bool nonLeaf = (flags & PAGE_NONLEAF) != 0;

  // Thành phần không hiện diện: trả về sạch tuyệt đối, không giữ lại địa chỉ vật lý
  // hay bất kỳ bit nào khác. Nếu sau này cần lưu siêu dữ liệu (swap slot...)
  // cho not-present PTE, xử lý ở một hàm/đường dẫn riêng, không lẫn vào đây.
  if (!(flags & PAGE_PRESENT))
  {
    return 0;
  }

#if defined(__x86_64__)
  pte  = physicalAddress & 0x000ffffffffff000ull;
  pte |= (1ull << 0); // P

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

#elif defined(__aarch64__)
  pte = physicalAddress & 0x0000fffffffff000ull;

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

#elif defined(__riscv)
  pte  = ((physicalAddress >> 12) & 0xFFFFFFFFFFFull) << 10;
  pte |= (1ull << 0); // V

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

#else
#error "Dòng vi xử lý này chưa được VNExos hỗ trợ!"
#endif

  return pte;
}

PageTable* allocateZeroPageTable()
{
  if (start2ndMaxRegion + 0x1000 > end2ndMaxRegion)
    return nullptr;

  PageTable* page = reinterpret_cast<PageTable*>(start2ndMaxRegion);

  start2ndMaxRegion += 0x1000;
  memset(page, 0, 0x1000);

  return page;
}

void* allocatePages(uint64_t numberOfPages)
{
  if (ptrMaxRegion + numberOfPages * 0x1000ull > endMaxRegion)
    return nullptr;

  void* pages   = (void*)ptrMaxRegion;
  ptrMaxRegion += 0x1000ull * numberOfPages;

  return pages;
}

void clearPages()
{
  ptrMaxRegion = startMaxRegion;
}
