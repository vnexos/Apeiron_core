/**
 * Copyright (c) 2026 VNExos Inc.
 * Bảo lưu mọi quyền.
 *
 * Được cấp phép theo Giấy phép Độc quyền.
 * Xem tệp LICENSE tại thư mục `internal` để biết thêm chi
 * tiết.
 *
 * @file main.cpp
 * @brief Tệp khởi đầu của Bộ nạp khởi động
 */
#include <common.hpp>
#include <cpu.hpp>
#include <efilib.hpp>

using namespace EFI;

// TODO: Khi làm memory map thì kiếm phân vùng lớn nhất rồi đẩy rác vào cuối và lùi dần từng trang như stack để sau này dễ bề quản lý
extern "C" [[gnu::ms_abi]] EFI_STATUS
vnexos_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
  EFI_BOOT_SERVICES* bs = SystemTable->BootServices;
  init(ImageHandle, SystemTable);

  /* Dùng giao thức ảnh đã tải để lấy các tham số được truyền từ bộ nạp mồi */
  EFI_LOADED_IMAGE_PROTOCOL* lip;
  EFI_GUID                   lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;

  EFI_STATUS status = bs->OpenProtocol(
      ImageHandle,
      &lipGuid,
      (void**)&lip,
      ImageHandle, nullptr,
      EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (EFI_ERROR(status))
  {
    printf("LOI: Khong the mo giao thuc anh da tai\nNhan phim bat ky de thoat...");
    waitForKey();
    printf("\n");
    return status;
  }

  if (lip->LoadOptionsSize != sizeof(ApeironCommonParameters))
  {
    printf("LOI: Tham so truyen vao khong dung (mong doi: %d byte, nhung co: %d byte)\n\
      Nhan phim bat ky de thoat...",
           sizeof(ApeironCommonParameters), lip->LoadOptionsSize);
    waitForKey();
    printf("\n");
    return status;
  }

  ApeironCommonParameters* params = (ApeironCommonParameters*)lip->LoadOptions;

  printf("Test framebuffer: 0x%16x\n", params->framebuffer);

  /* Tính toán vị trí đẹp cho thanh chờ */

  while (true)
  {
    cpu_halt();
  }

  return EFI_SUCCESS;
}
