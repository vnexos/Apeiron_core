#include <cpu.hpp>

extern "C" void kernel_main()
{
  while (true)
  {
    cpu_halt();
  }
}