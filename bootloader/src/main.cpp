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

#include "graphics.hpp"

using namespace EFI;

struct LoadingBlock
{
  uint32_t x, y;
  uint32_t w, h;
};

struct LoadingBarStatus
{
  LoadingBlock                  block;
  EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
  EFI_BOOT_SERVICES*            bs;
  uint64_t                      progress;
};

void drawFilledCapsule(
    EFI_GRAPHICS_OUTPUT_PROTOCOL*  gop,
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* circleBuffer,
    uint32_t                       width,
    uint32_t                       height,
    uint32_t                       posX,
    uint32_t                       posY,
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL* primaryColor)
{
  if (width < height)
    width = height;

  uint32_t capWidth = (height + 1) / 2;

  // Tính toán Delta (kích thước tính bằng byte của một hàng trong circleBuffer)
  // circleBuffer là hình vuông có cạnh = height, do đó mỗi hàng dài height pixel.
  uint32_t delta = height * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL);

  // Nửa bên trái
  if (capWidth > 0)
    gop->Blt(gop, circleBuffer, EfiBltBufferToVideo, 0, 0, posX, posY, capWidth, height, delta);

  // Nửa bên phải (sẽ chồng lên pixel trung tâm của nửa trái nếu height là số lẻ)
  uint32_t rightCapX       = posX + width - capWidth;
  uint32_t rightCapSourceX = height - capWidth;
  if (capWidth > 0)
    gop->Blt(gop, circleBuffer, EfiBltBufferToVideo, rightCapSourceX, 0, rightCapX, posY, capWidth, height, delta);

  // Khối chữ nhật ở giữa
  if (width > 2 * capWidth)
  {
    uint32_t rectWidth = width - 2 * capWidth;
    gop->Blt(gop, primaryColor, EfiBltVideoFill, 0, 0, posX + capWidth, posY, rectWidth, height, 0);
  }
}

void EFI_API drawProgressBar(EFI_EVENT evt, void* context)
{
  LoadingBarStatus*             status     = (LoadingBarStatus*)context;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL background = {
      0, 0, 0, 0};
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL track = {
      128, 128, 128, 0};
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL primary = {
      255, 255, 255, 0};

  EFI_GRAPHICS_OUTPUT_BLT_PIXEL* circleBuffer;
  status->bs->AllocatePool(EfiLoaderData, status->block.h * status->block.h * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL), (void**)&circleBuffer);
  drawFilledCircle(circleBuffer, status->block.h, &track, &background);
  drawFilledCapsule(status->gop, circleBuffer, status->block.w, status->block.h, status->block.x, status->block.y, &track);
  drawFilledCircle(circleBuffer, status->block.h - 4, &primary, &track);
  drawFilledCapsule(status->gop, circleBuffer, status->block.h - 4 + (status->block.w - status->block.h) * status->progress / 100, status->block.h - 4, status->block.x + 2, status->block.y + 2, &primary);
  status->bs->FreePool(circleBuffer);

  status->progress++;
  if (status->progress >= 100)
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
      500000);
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

    SetupTimer(bs, &loadingStatus);
  }

  while (true)
  {
    cpu_halt();
  }

  return EFI_SUCCESS;
}
