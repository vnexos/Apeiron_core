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
#include <cpu.hpp>
#include <efi.hpp>

// TODO: Khi làm memory map thì kiếm phân vùng lớn nhất rồi đẩy rác vào cuối và lùi dần từng trang như stack để sau này dễ bề quản lý
extern "C" [[gnu::ms_abi]] EFI_STATUS
vnexos_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
  (void)ImageHandle;
  SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
  SystemTable->ConOut->OutputString(SystemTable->ConOut, EFI_TEXT("Hello world!\r\n"));
  while (true)
  {
    cpu_halt();
  }
  return EFI_SUCCESS;
}
