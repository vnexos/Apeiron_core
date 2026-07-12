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
#include <string.hpp>

#include "graphics.hpp"

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
};

void EFI_API drawProgressBar(EFI_EVENT evt, void* context)
{
  LoadingBarStatus*             status     = (LoadingBarStatus*)context;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL background = {
      0, 0, 0, 0};
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL track = {
      128, 128, 128, 0};
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL primary = {
      255, 255, 255, 0};

  memset(status->buffer, 0, status->block.w * status->block.h * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
  drawFilledCapsule(status->buffer, status->block.w, status->block.h, &track);
  drawFilledCapsule(status->buffer, status->block.h + (status->block.w - status->block.h) * status->progress / 100, status->block.h, &primary, status->block.w);

  status->gop->Blt(
      status->gop, status->buffer,
      EfiBltBufferToVideo,
      0, 0,
      status->block.x, status->block.y,
      status->block.w, status->block.h,
      0);

  status->progress++;
  if (status->progress > 100)
    status->progress = 0;
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

  ApeironCommonParameters* params        = (ApeironCommonParameters*)lip->LoadOptions;
  LoadingBarStatus         loadingStatus = {
      {0, 0, 0, 0},
      params->graphicsOutputProtocol,
      SystemTable->BootServices,
      0};

  printf("SCREEN: %d %d \n", params->horizontalResolution, params->verticalResolution);

  if (params->graphicsOutputProtocol)
  {
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = params->graphicsOutputProtocol;

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

    printf("POS : %d - %d\nSIZE: %d - %d\n", (int)loadingBarXOffset, (int)loadingBarYOffset, (int)loadingBarWidth, (int)loadingBarHeight);

    loadingStatus.block = {loadingBarXOffset, loadingBarYOffset, loadingBarWidth, loadingBarHeight};
    bs->AllocatePool(EfiLoaderData, loadingBarWidth * loadingBarHeight * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL), (void**)&loadingStatus.buffer);

    SetupTimer(bs, &loadingStatus);
  }

  while (true)
  {
    cpu_halt();
  }

  return EFI_SUCCESS;
}
