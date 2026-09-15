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

#if defined(__x86_64__)
#include <hal/mmx64.hpp>
#elif defined(__aarch64__)
#include <hal/mmaa64.hpp>
#elif defined(__riscv)
#include <hal/mmriscv64.hpp>
#else
#error "Dòng vi xử lý này chưa được VNExos hỗ trợ!"
#endif

using namespace EFI;

/* TRANG ĐẦU VÀ TRANG CUỐI CỦA VÙNG NHỚ LỚN NHẤT */
uint64_t startMaxRegion    = 0;
uint64_t ptrMaxRegion      = 0;
uint64_t endMaxRegion      = 0;
uint64_t start2ndMaxRegion = 0;
uint64_t ptr2ndMaxRegion   = 0;
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

    if (((EFI_MEMORY_DESCRIPTOR*)maxDesc)->NumberOfPages < desc->NumberOfPages)
      secondMaxDesc = maxDesc, maxDesc = ptr;
    else if (((EFI_MEMORY_DESCRIPTOR*)secondMaxDesc)->NumberOfPages < desc->NumberOfPages)
      secondMaxDesc = ptr;

    ptr += descriptorSize;
  }

  EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)maxDesc;
  endMaxRegion                = desc->PhysicalStart + (desc->NumberOfPages * 0x1000ull);
  startMaxRegion              = desc->PhysicalStart;
  ptrMaxRegion                = startMaxRegion;

  desc              = (EFI_MEMORY_DESCRIPTOR*)secondMaxDesc;
  end2ndMaxRegion   = desc->PhysicalStart + (desc->NumberOfPages * 0x1000ull);
  start2ndMaxRegion = desc->PhysicalStart;
  ptr2ndMaxRegion   = start2ndMaxRegion;

  return true;
}

uint64_t makePte(uint64_t physicalAddress, uint64_t flags)
{
  const bool nonLeaf = (flags & PAGE_NONLEAF) != 0;

  // Thành phần không hiện diện: trả về sạch tuyệt đối, không giữ lại địa chỉ vật lý
  // hay bất kỳ bit nào khác. Nếu sau này cần lưu siêu dữ liệu (swap slot...)
  // cho not-present PTE, xử lý ở một hàm/đường dẫn riêng, không lẫn vào đây.
  if (!(flags & PAGE_PRESENT))
  {
    return 0;
  }

  return archMakePte(physicalAddress, flags, nonLeaf);
}

PageTable* allocateZeroPageTable()
{
  if (ptr2ndMaxRegion + 0x1000 > end2ndMaxRegion)
    return nullptr;

  PageTable* page = reinterpret_cast<PageTable*>(ptr2ndMaxRegion);

  ptr2ndMaxRegion += 0x1000;
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

uint64_t toVirtualAddress(uint16_t rootIndex, uint16_t upperIndex, uint16_t middleIndex, uint16_t lowerIndex)
{
  return archToVirtualAddress(rootIndex, upperIndex, middleIndex, lowerIndex);
}
