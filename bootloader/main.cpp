#include <cpu.hpp>
#include <efi.hpp>

extern "C" [[gnu::ms_abi]] EFI_STATUS
vnexos_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
  SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
  SystemTable->ConOut->OutputString(SystemTable->ConOut, EFI_TEXT("Hello world!"));
  while (true)
  {
    cpu_halt();
  }
  return EFI_SUCCESS;
}
