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

int* a = (int*)0xffab;
int* b = (int*)0xccdd;

int add(int a, int b)
{
  return a + b;
}

void entry()
{
  while (true)
  {
    cpu_halt();
  }
}