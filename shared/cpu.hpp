/**
 * Copyright (c) 2026 VNExos Inc.
 * Bảo lưu mọi quyền.
 *
 * Được cấp phép theo Giấy phép Độc quyền.
 * Xem tệp LICENSE tại thư mục `internal` để biết thêm chi
 * tiết.
 *
 * @file cpu.hpp
 * @brief Những câu lện đơn giản để tương tác với CPU
 */
#ifndef __SHARED__CPU_HPP
#define __SHARED__CPU_HPP

inline void cpu_halt()
{
#if defined(__x86_64__)
  __asm__ __volatile__("hlt");
#elif defined(__aarch64__)
  // Trạng thái nghỉ tiết kiệm điện của ARM, chờ ngắt từ GIC (Generic Interrupt Controller)
  __asm__ __volatile__("wfi");
#elif defined(__riscv)
  // Trạng thái nghỉ của RISC-V, chờ ngắt từ PLIC/CLINT
  __asm__ __volatile__("wfi");
#else
#error "Kiến trúc chip này chưa được VNExos hỗ trợ!"
#endif
}

#endif // __SHARED__CPU_HPP
