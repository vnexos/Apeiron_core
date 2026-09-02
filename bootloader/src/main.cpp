/**
 * Copyright (c) 2026 VNExos
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
#include <post_quantum/sign.hpp>
#include <string.hpp>

#include <graphics.hpp>
#include <mm.hpp>

#if defined(__x86_64__)
#define BOOT_FILE EFI_TEXT("\\EFI\\BOOT\\BOOTX64.EFI")
#elif defined(__aarch64__)
#define BOOT_FILE EFI_TEXT("\\EFI\\BOOT\\BOOTAA64.EFI")
#elif defined(__riscv)
#define BOOT_FILE EFI_TEXT("\\EFI\\BOOT\\BOOTRISCV64.EFI")
#else
#error "Dòng vi xử lý này chưa được VNExos hỗ trợ!"
#endif

using namespace EFI;

struct LoadingBlock
{
  uint32_t x, y;
  uint32_t w, h;
};

struct LoadingBarStatus
{
  LoadingBlock                   block;
  EFI_GRAPHICS_OUTPUT_PROTOCOL*  gop;
  EFI_BOOT_SERVICES*             bs;
  uint64_t                       progress;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL* buffer;
  bool                           bStopFlag = false;
};

void EFI_API drawProgressBar(EFI_EVENT evt, void* context)
{
  (void)evt;
  LoadingBarStatus*             status = (LoadingBarStatus*)context;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL track  = {
      128, 128, 128, 0};
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL primary = {
      255, 255, 255, 0};

  memset(status->buffer, 0, status->block.w * status->block.h * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  drawFilledCapsule(status->buffer, status->block.w, status->block.h, &track);
  uint32_t maxWidth = status->block.w * 20 / 100;

  uint32_t totalVirtualRange = status->block.w + maxWidth - 2 * status->block.h;

  // Áp dụng thuật toán Ease-in-out (Smoothstep) bằng số nguyên
  // Công thức: f(x) = x^2 * (3 - 2x)
  // Tạo hiệu ứng Ping-pong dội qua lại (0 -> 100 -> 0)
  uint32_t p       = status->progress <= 100 ? status->progress : 200 - status->progress;
  uint32_t p_eased = (p * p * (300 - 2 * p)) / 10000;

  uint32_t virtualX = status->block.h + (totalVirtualRange * p_eased) / 100;

  uint32_t xRight    = virtualX > status->block.w ? status->block.w : virtualX;
  int32_t  xLeftCalc = (int32_t)virtualX - (int32_t)maxWidth;
  uint32_t xLeft     = xLeftCalc < 0 ? 0 : xLeftCalc;

  uint32_t width = xRight - xLeft;
  if (width < status->block.h) width = status->block.h;
  drawFilledCapsule(status->buffer, width, status->block.h, &primary, status->block.w, xLeft);

  status->gop->Blt(
      status->gop, status->buffer,
      EfiBltBufferToVideo,
      0, 0,
      status->block.x, status->block.y,
      status->block.w, status->block.h,
      0);

  status->progress++;
  if (status->progress >= 200)
  {
    status->progress  = 0;
    status->bStopFlag = true;
  } else if (status->bStopFlag)
  {
    status->bStopFlag = false;
  }
}

EFI_EVENT SetupTimer(EFI_BOOT_SERVICES* bs, LoadingBarStatus* context)
{
  EFI_EVENT  timerEvent;
  EFI_STATUS status;

  status = bs->CreateEvent(
      EVT_TIMER | EVT_NOTIFY_SIGNAL,
      TPL_CALLBACK,
      drawProgressBar,
      context,
      &timerEvent);
  if (EFI_ERROR(status))
    return nullptr;

  status = bs->SetTimer(
      timerEvent,
      TimerPeriod,
      5000);
  if (EFI_ERROR(status))
    return nullptr;

  return timerEvent;
}

void clearTimer(EFI_BOOT_SERVICES* bs, LoadingBarStatus* loadingStatus, EFI_EVENT timerEvent)
{
  if (timerEvent)
  {
    while (!loadingStatus->bStopFlag)
    {
      bs->Stall(0);
    }
    bs->SetTimer(timerEvent, TimerCancel, 0);
  }
}

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
    printf("LOI [1]: Khong the mo giao thuc anh da tai\nNhan phim bat ky de thoat...");
    waitForKey();
    printf("\n");
    return status;
  }

  if (lip->LoadOptionsSize != sizeof(ApeironCommonParameters))
  {
    printf("LOI [2]: Tham so truyen vao khong dung (mong doi: %d byte, nhung co: %d byte)\n\
      Nhan phim bat ky de thoat...",
           sizeof(ApeironCommonParameters), lip->LoadOptionsSize);
    waitForKey();
    printf("\n");
    return status;
  }

  ApeironCommonParameters* params        = (ApeironCommonParameters*)lip->LoadOptions;
  LoadingBarStatus         loadingStatus = {
      {0, 0, 0, 0},
      params->graphicsOutputProtocol,
      SystemTable->BootServices,
      0,
      nullptr};

  EFI_EVENT timerEvent = nullptr;
  if (params->graphicsOutputProtocol)
  {
    /* Tính toán vị trí đẹp cho thanh chờ */
    uint32_t loadingBarWidth;
    if (params->verticalResolution > params->horizontalResolution)
      loadingBarWidth = params->horizontalResolution * 50 / 100;
    else
      loadingBarWidth = params->horizontalResolution * 25 / 100;
    uint32_t loadingBarHeight = (int)((params->verticalResolution) * 1 / 100);

    uint32_t loadingBarXOffset = (params->horizontalResolution - loadingBarWidth) / 2;
    uint32_t loadingBarYOffset = 0;
    if (params->OEMLogoSize == 0)
    {
      uint32_t logoBottom = UNPACK_HIGH_32(params->logoPosition) + UNPACK_HIGH_32(params->logoSize);

      uint32_t loadingBarSegment = (params->verticalResolution - logoBottom) / 5;

      uint32_t loadingBarTopSegmentPosition = logoBottom + loadingBarSegment;

      loadingBarYOffset = loadingBarTopSegmentPosition + (loadingBarSegment - loadingBarHeight) / 2;
    } else
    {
      uint32_t OEMLogoBottom = UNPACK_HIGH_32(params->OEMLogoSize) + UNPACK_HIGH_32(params->OEMLogoPosition);
      uint32_t VNExosLogoTop = UNPACK_HIGH_32(params->logoPosition);

      if (VNExosLogoTop > OEMLogoBottom && (VNExosLogoTop - OEMLogoBottom) >= loadingBarHeight)
      {
        uint32_t loadingBarSegment = (VNExosLogoTop - OEMLogoBottom) / 2;

        uint32_t loadingBarBottomSegmentPosition = VNExosLogoTop - loadingBarSegment;

        loadingBarYOffset = loadingBarBottomSegmentPosition + (loadingBarSegment - loadingBarHeight) / 2;
      } else
      {
        loadingBarYOffset = params->verticalResolution - loadingBarHeight - 10;
      }
    }

    loadingStatus.block = {loadingBarXOffset, loadingBarYOffset, loadingBarWidth, loadingBarHeight};
    bs->AllocatePool(EfiLoaderData, loadingBarWidth * loadingBarHeight * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL), (void**)&loadingStatus.buffer);

    timerEvent = SetupTimer(bs, &loadingStatus);
  }

  /* Đọc khóa công khai vào bộ nhớ */
  uint8_t* keyBuffer;
  uint64_t keySize;
  status = loadFile(EFI_TEXT("\\certs\\root.crt"), &keyBuffer, &keySize);
  if (EFI_ERROR(status))
  {
    printf("LOI [3]: Khong the doc tep: %ws\nNhan phim bat ky de thoat...", EFI_TEXT("\\certs\\root.crt"));
    waitForKey();
    printf("\n");
    clearTimer(bs, &loadingStatus, timerEvent);
    return status;
  }

  /* Đọc khóa phụ vào bộ nhớ */
  uint8_t* secondKeyBuffer;
  uint64_t secondKeySize;
  status = loadFile(EFI_TEXT("\\certs\\open.crt"), &secondKeyBuffer, &secondKeySize);
  if (EFI_ERROR(status))
  {
    printf("LOI [4]: Khong the doc tep: %ws\nNhan phim bat ky de thoat...", EFI_TEXT("\\certs\\open.crt"));
    waitForKey();
    printf("\n");
    clearTimer(bs, &loadingStatus, timerEvent);
    return status;
  }

  /* Đọc tệp bộ nạp khởi động mồi vào bộ nhớ */
  uint8_t* bootBuffer;
  uint64_t bootSize;
  status = loadFile(BOOT_FILE, &bootBuffer, &bootSize);
  if (EFI_ERROR(status))
  {
    printf("LOI [5]: Khong the doc tep: %ws\nNhan phim bat ky de thoat...", BOOT_FILE);
    waitForKey();
    printf("\n");
    clearTimer(bs, &loadingStatus, timerEvent);
    return status;
  }

  /* Tiến hành xác thực ngược chữ ký */
  bool bOriginalBoot;
  if (Sign::verifyEfiFileSignature(bootBuffer, bootSize, keyBuffer, keySize))
  {
    bOriginalBoot = true;
  } else if (Sign::verifyEfiFileSignature(bootBuffer, bootSize, secondKeyBuffer, secondKeySize))
  {
    bOriginalBoot = false;
  } else
  {
    printf("LOI [6]: Chu ky khong hop le: %ws\nNhan phim bat ky de thoat...", BOOT_FILE);
    waitForKey();
    printf("\n");
    clearTimer(bs, &loadingStatus, timerEvent);
    return status;
  }

  (void)bOriginalBoot;
  // if (bOriginalBoot)
  //   printf("Nhan goc!\n"); // Nhân gốc
  // else
  //   printf("Nhan mo!\n");  // Nhân mở

  if (!initMemoryManagement(bs))
  {
    clearTimer(bs, &loadingStatus, timerEvent);
    return -1;
  }

  waitForKey();

  clearTimer(bs, &loadingStatus, timerEvent);

  while (true)
  {
    cpu_halt();
  }

  return EFI_SUCCESS;
}
