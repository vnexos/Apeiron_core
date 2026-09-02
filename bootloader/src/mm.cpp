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

using namespace EFI;

/* TRANG ĐẦU VÀ TRANG CUỐI CỦA VÙNG NHỚ LỚN NHẤT */
uint64_t startRegion = 0;
uint64_t endRegion   = 0;

bool initMemoryManagement(EFI_BOOT_SERVICES* BootServices)
{
  EFI_STATUS             status;
  EFI_MEMORY_DESCRIPTOR* map     = nullptr;
  uint64_t               mapSize = 0;
  uint64_t               mapKey;
  uint64_t               descriptorSize;
  uint32_t               descriptorVersion;

  status = BootServices->GetMemoryMap(&mapSize, map, &mapKey, &descriptorSize, &descriptorVersion);

  mapSize += 2 * descriptorSize;

  status = BootServices->AllocatePool(EfiLoaderData, mapSize, (void**)&map);
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

  uint64_t numEntries = mapSize / descriptorSize;
  uint8_t* maxDesc    = (uint8_t*)map;
  uint8_t* ptr        = maxDesc + descriptorSize;

  for (uint64_t i = 1; i < numEntries; ++i)
  {
    EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)ptr;
    if (desc->Type != EfiConventionalMemory)
    {
      ptr += descriptorSize;
      continue;
    }

    if (((EFI_MEMORY_DESCRIPTOR*)maxDesc)->NumberOfPages < desc->NumberOfPages)
      maxDesc = ptr;

    ptr += descriptorSize;
  }
  EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)maxDesc;

  endRegion = desc->PhysicalStart + (desc->NumberOfPages * 0x1000);

  printf("Max Memory Section:\n");

  printf("- Loai: %d \n", desc->Type);
  printf("- Vung nho: %x - %x (%d)\n", desc->PhysicalStart, endRegion, desc->NumberOfPages);

  startRegion = desc->PhysicalStart;

  return true;
}