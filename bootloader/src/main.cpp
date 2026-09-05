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
#include <post_quantum/crypto/aes256.hpp>
#include <post_quantum/crypto/sha3.hpp>
#include <post_quantum/kem/kyber.hpp>
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

typedef struct __attribute__((packed))
{
  uint8_t  hash[32];
  uint16_t fileName[256];
} FileHash;

// Xóa sạch vùng nhớ nhạy cảm
inline void secureZeroize(void* p, uint64_t n)
{
  volatile uint8_t* vp = (volatile uint8_t*)p;
  while (n--)
    *vp++ = 0;
}

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

EFI_STATUS hashFilesInFolder(EFI_BOOT_SERVICES* bs, EFI_FILE_PROTOCOL* dirHandle, const uint16_t* prefix, uint8_t* hash)
{
  EFI_STATUS     status;
  uint64_t       bufferSize;
  uint64_t       prefixSize;
  EFI_FILE_INFO* fileInfo;

  uint64_t capacity = 1;
  uint64_t count    = 0;

  if (dirHandle == nullptr || prefix == nullptr || hash == nullptr)
  {
    printf("LOI: Tham so dau vao khong hop le.");
    return 1;
  }

  prefixSize = wstrlen(prefix);
  bufferSize = sizeof(EFI_FILE_INFO) + 256 * sizeof(uint16_t);
  status     = bs->AllocatePool(EfiLoaderData, bufferSize, (void**)&fileInfo);
  if (EFI_ERROR(status))
  {
    printf("LOI: Cap phat that bai.");
    return status;
  }

  // Khởi tạo mảng thông tin hash
  FileHash* hashInfoBuffer;
  status = bs->AllocatePool(EfiLoaderData, capacity * sizeof(FileHash), (void**)&hashInfoBuffer);
  if (EFI_ERROR(status))
  {
    printf("LOI: Cap phat that bai.");
    bs->FreePool(fileInfo);
    return status;
  }

  dirHandle->SetPosition(dirHandle, 0);

  while (true)
  {
    uint64_t readSize = bufferSize;
    status            = dirHandle->Read(dirHandle, &readSize, fileInfo);

    if (EFI_ERROR(status) || readSize == 0)
    {
      break;
    }

    // Bỏ qua . và ..
    if (wstrcmp(fileInfo->FileName, (uint16_t*)L".") == 0 || wstrcmp(fileInfo->FileName, (uint16_t*)L"..") == 0)
    {
      continue;
    }

    // Bỏ qua thư mục con
    if (fileInfo->Attribute & EFI_FILE_DIRECTORY)
    {
      continue;
    }

    // Kiểm tra tiền tố
    bool match = false;
    if (wstrlen(fileInfo->FileName) >= prefixSize)
      match = wstrcmp(fileInfo->FileName, prefix, prefixSize) == 0;

    if (match)
    {
      if (count >= capacity)
      {
        uint64_t  newCapacity = capacity * 2;
        FileHash* newHashInfoBuffer;
        status = bs->AllocatePool(EfiLoaderData, newCapacity * sizeof(FileHash), (void**)&newHashInfoBuffer);
        if (EFI_ERROR(status))
          break;

        memcpy(newHashInfoBuffer, hashInfoBuffer, capacity * sizeof(FileHash));
        bs->FreePool(hashInfoBuffer);

        capacity       = newCapacity;
        hashInfoBuffer = newHashInfoBuffer;
      }

      EFI_FILE_PROTOCOL* fileHandle;
      status = dirHandle->Open(dirHandle, &fileHandle, (uint16_t*)fileInfo->FileName, EFI_FILE_MODE_READ, 0);
      if (EFI_ERROR(status))
        break;

      // Cấp phát kích thước bằng đúng kích thước tệp
      uint64_t fileSize = fileInfo->FileSize;
      uint8_t* fileBuffer;

      uint64_t fileNameLen = wstrlen(fileInfo->FileName) + 1;
      memcpy(hashInfoBuffer[count].fileName, fileInfo->FileName, fileNameLen * sizeof(uint16_t));

      printf("%d - %ws\n", fileNameLen, hashInfoBuffer[count].fileName);

      if (fileSize > 0)
      {
        status = bs->AllocatePool(EfiLoaderData, fileSize, (void**)&fileBuffer);
        if (EFI_ERROR(status))
        {
          fileHandle->Close(fileHandle);
          break;
        }

        // Đọc tệp vào bộ nhớ
        status = fileHandle->Read(fileHandle, &fileSize, fileBuffer);
        fileHandle->Close(fileHandle);
        if (EFI_ERROR(status))
        {
          bs->FreePool(fileBuffer);
          break;
        }

        // Băm tệp vào bộ nhớ
        Crypto::VNExos::sha256(hashInfoBuffer[count].hash, fileBuffer, fileSize);
        bs->FreePool(fileBuffer);
      } else
      {
        fileHandle->Close(fileHandle);
        Crypto::VNExos::sha256(hashInfoBuffer[count].hash, (uint8_t*)"", 0);
      }

      ++count;
    }
  }

  bs->FreePool(fileInfo);

  // Trong trường hợp ko có tệp nào
  if (count == 0)
  {
    bs->FreePool(hashInfoBuffer);
    Crypto::VNExos::sha256(hash, (uint8_t*)"", 0);
    return EFI_SUCCESS;
  }

  // Khởi tạo mảng nối các mã băm của các tệp
  uint8_t* hashBuffer;
  status = bs->AllocatePool(EfiLoaderData, count * 32, (void**)&hashBuffer);
  if (EFI_ERROR(status))
  {
    printf("LOI: Cap phat that bai.");
    bs->FreePool(hashInfoBuffer);
    return status;
  }

  // Sắp xếp các bảng băm theo thứ tự của tên
  FileHash tmpHash;
  for (uint64_t i = 0; i < count - 1; ++i)
  {
    for (uint64_t j = i + 1; j < count; ++j)
    {
      if (wstrcmp(hashInfoBuffer[i].fileName, hashInfoBuffer[j].fileName) > 0)
      {
        memcpy(&tmpHash, &hashInfoBuffer[i], sizeof(FileHash));
        memcpy(&hashInfoBuffer[i], &hashInfoBuffer[j], sizeof(FileHash));
        memcpy(&hashInfoBuffer[j], &tmpHash, sizeof(FileHash));
      }
    }
  }

  // Đẩy các mã băm ra bộ đệm
  for (uint64_t i = 0; i < count; ++i)
  {
    memcpy(hashBuffer + i * 32, hashInfoBuffer[i].hash, 32);
  }

  bs->FreePool(hashInfoBuffer);

  // Băm bộ đệm một lần nữa
  Crypto::VNExos::sha256(hash, hashBuffer, count * 32);
  bs->FreePool(hashBuffer);

  return status;
}

uint64_t setupPaging(EFI_MEMORY_DESCRIPTOR* map, uint64_t mapSize, uint64_t descriptorSize)
{
  PageTable* rootTable = allocateZeroPageTable();

  // Nửa trên (kernel-half)
  {
    PageTable* upperTable   = allocateZeroPageTable();
    rootTable->entries[256] = makePte((uint64_t)upperTable, PAGE_PRESENT | PAGE_NONLEAF);

    PageTable* middleTable = allocateZeroPageTable();
    upperTable->entries[0] = makePte((uint64_t)middleTable, PAGE_PRESENT | PAGE_NONLEAF);

    PageTable* lowerTable   = allocateZeroPageTable();
    middleTable->entries[0] = makePte((uint64_t)lowerTable, PAGE_PRESENT | PAGE_NONLEAF);
  }

  return (uint64_t)rootTable;
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
  bs->FreePool(bootBuffer);
  bs->FreePool(secondKeyBuffer);

  // Lấy hash của các tệp
  EFI_FILE_PROTOCOL* dirHandle;
  uint8_t*           hash;
  status = loadDir(EFI_TEXT("\\EFI\\BOOT"), &dirHandle);
  if (EFI_ERROR(status))
  {
    printf("LOI [7]: Khong the doc thu muc: %ws\nNhan phim bat ky de thoat...", EFI_TEXT("\\EFI\\BOOT"));
    waitForKey();
    printf("\n");
    clearTimer(bs, &loadingStatus, timerEvent);
    return status;
  }

  // Khởi tạo mảng băm chung
  status = bs->AllocatePool(EfiLoaderData, 3 * 32, (void**)&hash);
  if (EFI_ERROR(status))
  {
    printf("LOI [8]: Cap phat that bai: hash\nNhan phim bat ky de thoat...");
    waitForKey();
    printf("\n");
    clearTimer(bs, &loadingStatus, timerEvent);
    return status;
  }

  // Băm toàn bộ tệp BOOT: SHAV256(SHAV256(BOOTX64.EFI) || ...)
  status = hashFilesInFolder(bs, dirHandle, EFI_TEXT("BOOT"), hash);
  if (EFI_ERROR(status))
  {
    printf("LOI [9]: Khong the bam cac tep BOOT* trong thu muc: %ws\nNhan phim bat ky de thoat...", EFI_TEXT("\\EFI\\BOOT"));
    waitForKey();
    printf("\n");
    clearTimer(bs, &loadingStatus, timerEvent);
    return status;
  }

  // Băm toàn bộ tệp vnexos: SHAV256(SHAV256(vnexosx64.efi) || ...)
  status = hashFilesInFolder(bs, dirHandle, EFI_TEXT("vnexos"), hash + 32);
  if (EFI_ERROR(status))
  {
    printf("LOI [10]: Khong the bam cac tep vnexos* trong thu muc: %ws\nNhan phim bat ky de thoat...", EFI_TEXT("\\EFI\\BOOT"));
    waitForKey();
    printf("\n");
    clearTimer(bs, &loadingStatus, timerEvent);
    return status;
  }

  // Băm tệp nhân lõi
  uint8_t* kernelBuffer;
  uint64_t kernelSize;
  status = loadFile(EFI_TEXT("\\apeiron.kern"), &kernelBuffer, &kernelSize);
  if (EFI_ERROR(status))
  {
    printf("LOI [11]: Khong the doc tep: %ws\nNhan phim bat ky de thoat...", EFI_TEXT("\\apeiron.kern"));
    waitForKey();
    printf("\n");
    clearTimer(bs, &loadingStatus, timerEvent);
    return status;
  }

  USXSecurity* secTable = Sign::verifyUsxFileSignature(kernelBuffer, kernelSize, keyBuffer, keySize);
  if (!secTable)
  {
    printf("LOI [12]: Tep USX khong hop le: %ws\nNhan phim bat ky de thoat...", EFI_TEXT("\\apeiron.kern"));
    waitForKey();
    printf("\n");
    clearTimer(bs, &loadingStatus, timerEvent);
    return status;
  }

  printf("0x%x - 0x%d\n", secTable->KEMOffset, secTable->KEMSize);

  Crypto::VNExos::sha256(hash + 64, kernelBuffer, kernelSize);

  Crypto::VNExos::sha256(hash + 32, hash, 96);
  Crypto::VNExos::sha256(hash, keyBuffer, keySize); // Băm tệp khóa chính

  Crypto::VNExos::sha256(hash, hash, 64);           // Băm tất cả vào một khóa 32 Byte

  // Đọc và xác minh tệp khóa bí mật
  uint8_t* kyberKeyBuffer;
  uint64_t kyberKeySize;
  status = loadFile(EFI_TEXT("\\key.sec"), &kyberKeyBuffer, &kyberKeySize);
  if (EFI_ERROR(status))
  {
    printf("LOI [13]: Khong the doc tep: %ws\nNhan phim bat ky de thoat...", EFI_TEXT("\\key.sec"));
    waitForKey();
    printf("\n");
    clearTimer(bs, &loadingStatus, timerEvent);
    return status;
  }

  uint8_t magicByte[8] = {0, 0, 0, 'S', 'E', 'A', 'N', 'V'};
  uint8_t i;
  for (i = 0; i < 8; ++i)
  {
    if (kyberKeyBuffer[kyberKeySize - 8 + i] != magicByte[i])
      break;
  }

  if (i != 8)
  {
    printf("LOI [14]: Tep khoa bi mat khong hop le: %ws\nNhan phim bat ky de thoat...", EFI_TEXT("\\key.sec"));
    waitForKey();
    printf("\n");
    clearTimer(bs, &loadingStatus, timerEvent);
    return status;
  }

  // Giải mã khóa bí mật
  uint8_t* iv = &kyberKeyBuffer[kyberKeySize - 24];

  Crypto::AES256::AES256Context ctx;
  Crypto::AES256::init(&ctx, hash);
  Crypto::AES256::counter(&ctx, iv, kyberKeyBuffer, kyberKeyBuffer, kyberKeySize - 24);

  // Xóa dấu vết ngay sau khi dùng xong
  secureZeroize(hash, 96);
  bs->FreePool(hash);

  // Mở gói khóa
  uint8_t sharedSecret[32];
  Kyber::decapsulate(sharedSecret, kernelBuffer + secTable->KEMOffset, kyberKeyBuffer);
  secureZeroize(kyberKeyBuffer, kyberKeySize);

  for (uint8_t i = 0; i < 32; ++i)
    printf("%2x", sharedSecret[i]);
  printf("\n");

  (void)bOriginalBoot;
  // if (bOriginalBoot)
  //   printf("Nhan goc!\n"); // Nhân gốc
  // else
  //   printf("Nhan mo!\n");  // Nhân mở

  EFI_MEMORY_DESCRIPTOR* map;
  uint64_t               mapSize;
  uint64_t               descriptorSize;

  // if (!initMemoryManagement(bs, &map, &mapSize, &descriptorSize))
  // {
  //   clearTimer(bs, &loadingStatus, timerEvent);
  //   return -1;
  // }

  waitForKey();

  clearTimer(bs, &loadingStatus, timerEvent);

  while (true)
  {
    cpu_halt();
  }

  return EFI_SUCCESS;
}
