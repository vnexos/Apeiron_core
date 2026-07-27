/**
 * Copyright (c) 2026 VNExos
 * Bảo lưu mọi quyền.
 *
 * Được cấp phép theo Giấy phép Độc quyền.
 * Xem tệp LICENSE tại thư mục `internal` để biết thêm chi
 * tiết.
 *
 * @file main.cpp
 * @brief Tệp khởi đầu của Nhân lõi
 */
#include <cpu.hpp>
#include <usx.h>

USX_EXPORT_DATA int* a = (int*)0xffab;
USX_EXPORT_DATA int* b = (int*)0xccdd;

USX_EXPORT_FUNC int add(int a, int b)
{
  return a + b;
}

USX_IMPORT("libahihi", int, sub, int, int);

void entry()
{

  while (true)
  {
    cpu_halt();
  }
}