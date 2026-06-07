/**
 * @file efi.hpp
 * @brief Đây là cầu nối giữa mã nguồn C++ và
 * phần cứng thông qua firmware UEFI
 */
#ifndef __SHARED__EFI_HPP
#define __SHARED__EFI_HPP

#include <stdint.h>

#define EFI_API __attribute__((ms_abi))

/**************************************************************
 * ĐỊNH NGHĨA KIỂU DỮ LIỆU VÀ HẰNG SỐ
 **************************************************************/
typedef uint64_t EFI_STATUS;
typedef void*    EFI_HANDLE;
typedef void*    EFI_EVENT;
typedef uint64_t EFI_TPL;

#define EFI_SUCCESS          0
#define EFI_ERROR(status)    (status != EFI_SUCCESS)
#define EFI_TEXT(str)        ((uint16_t*)(L##str))
#define EFI_BUFFER_TOO_SMALL 0x8000000000000005

/**
 * GUID của Giao thức xuất giao diện của EFI là:
 * 9042A9DE-23DC-4A38-96FB-7ADED080516A
 */
#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID {           \
    0x9042a9de,                                       \
    0x23dc,                                           \
    0x4a38,                                           \
    {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}, \
}
/**
 * GUID của Loaded Image Protocol:
 * 5B1B31A1-9562-11D2-8E3F-00A0C969723B
 */
#define EFI_LOADED_IMAGE_PROTOCOL_GUID {              \
    0x5B1B31A1,                                       \
    0x9562,                                           \
    0x11D2,                                           \
    {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}, \
}
#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID {        \
    0x0964e5b22,                                      \
    0x6459,                                           \
    0x11d2,                                           \
    {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}, \
}

typedef struct
{
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t  Data4[8];
} EFI_GUID;

typedef struct
{
  uint8_t Type;
  uint8_t SubType;
  uint8_t Length[2];
} EFI_DEVICE_PATH_PROTOCOL;

typedef struct EFI_TIME
{
  uint16_t Year;
  uint8_t  Month;
  uint8_t  Day;
  uint8_t  Hour;
  uint8_t  Minute;
  uint8_t  Second;
  uint8_t  Pad1;
  uint32_t Nanosecond;
  int16_t  TimeZone;
  uint8_t  Daylight;
  uint8_t  PAD2;
} EFI_TIME;

/**************************************************************
 * GIAO THỨC TỆP
 **************************************************************/
#define EFI_FILE_INFO_ID                {0x09576e92, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}
#define EFI_FILE_SYSTEM_INFO_ID         {0x09576e93, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}
#define EFI_FILE_SYSTEM_VOLUME_LABEL_ID {0xdb47d7d3, 0xfe81, 0x11d3, {0x9a, 0x35, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d}}

#define EFI_FILE_PROTOCOL_REVISION        0x00010000
#define EFI_FILE_PROTOCOL_REVISION2       0x00020000
#define EFI_FILE_PROTOCOL_LATEST_REVISION EFI_FILE_PROTOCOL_REVISION2

#define EFI_FILE_MODE_READ   0x0000000000000001
#define EFI_FILE_MODE_WRITE  0x0000000000000002
#define EFI_FILE_MODE_CREATE 0x8000000000000000

#define EFI_FILE_READ_ONLY  0x0000000000000001
#define EFI_FILE_HIDDEN     0x0000000000000002
#define EFI_FILE_SYSTEM     0x0000000000000004
#define EFI_FILE_RESERVED   0x0000000000000008
#define EFI_FILE_DIRECTORY  0x0000000000000010
#define EFI_FILE_ARCHIVE    0x0000000000000020
#define EFI_FILE_VALID_ATTR 0x0000000000000037

typedef struct EFI_FILE_INFO
{
  uint64_t Size;
  uint64_t FileSize;
  uint64_t PhysicalSize;
  EFI_TIME CreateTime;
  EFI_TIME LastAccessTime;
  EFI_TIME ModificationTime;
  uint64_t Attribute;
  uint16_t FileName[];
} EFI_FILE_INFO;

typedef struct EFI_FILE_SYSTEM_INFO
{
  uint64_t Size;
  uint8_t  ReadOnly;
  uint64_t VolumeSize;
  uint64_t FreeSpace;
  uint32_t BlockSize;
  uint16_t VolumeLabel;
} EFI_FILE_SYSTEM_INFO;

typedef struct EFI_FILE_SYSTEM_VOLUME_LABEL
{
  uint16_t VolumeLabel[];
} EFI_FILE_SYSTEM_VOLUME_LABEL;

typedef struct EFI_FILE_IO_TOKEN
{
  EFI_EVENT  Event;
  EFI_STATUS Status;
  uint64_t   BufferSize;
  void*      Buffer;
} EFI_FILE_IO_TOKEN;

struct EFI_FILE_PROTOCOL;

typedef EFI_STATUS(EFI_API* EFI_FILE_OPEN)(struct EFI_FILE_PROTOCOL* This, struct EFI_FILE_PROTOCOL** NewHandle, uint16_t* FileName, uint64_t OpenMode, uint64_t Attributes);
typedef EFI_STATUS(EFI_API* EFI_FILE_CLOSE)(struct EFI_FILE_PROTOCOL* This);
typedef EFI_STATUS(EFI_API* EFI_FILE_DELETE)(struct EFI_FILE_PROTOCOL* This);
typedef EFI_STATUS(EFI_API* EFI_FILE_READ)(struct EFI_FILE_PROTOCOL* This, uint64_t* BufferSize, uint64_t* Buffer);
typedef EFI_STATUS(EFI_API* EFI_FILE_WRITE)(struct EFI_FILE_PROTOCOL* This, uint64_t* BufferSize, void* Buffer);
typedef EFI_STATUS(EFI_API* EFI_FILE_OPEN_EX)(struct EFI_FILE_PROTOCOL* This, struct EFI_FILE_PROTOCOL** NewHandle, uint16_t* FileName, uint64_t OpenMode, uint64_t Attributes, EFI_FILE_IO_TOKEN* Token);
typedef EFI_STATUS(EFI_API* EFI_FILE_READ_EX)(struct EFI_FILE_PROTOCOL* This, EFI_FILE_IO_TOKEN* Token);
typedef EFI_STATUS(EFI_API* EFI_FILE_WRITE_EX)(struct EFI_FILE_PROTOCOL* This, EFI_FILE_IO_TOKEN* Token);
typedef EFI_STATUS(EFI_API* EFI_FILE_FLUSH_EX)(struct EFI_FILE_PROTOCOL* This, EFI_FILE_IO_TOKEN* Token);
typedef EFI_STATUS(EFI_API* EFI_FILE_SET_POSITION)(struct EFI_FILE_PROTOCOL* This, uint64_t Position);
typedef EFI_STATUS(EFI_API* EFI_FILE_GET_POSITION)(struct EFI_FILE_PROTOCOL* This, uint64_t* Position);
typedef EFI_STATUS(EFI_API* EFI_FILE_GET_INFO)(struct EFI_FILE_PROTOCOL* This, EFI_GUID* InformationType, uint64_t* BufferSize, void* Buffer);
typedef EFI_STATUS(EFI_API* EFI_FILE_SET_INFO)(struct EFI_FILE_PROTOCOL* This, EFI_GUID* InformationType, uint64_t BufferSize, void* Buffer);
typedef EFI_STATUS(EFI_API* EFI_FILE_FLUSH)(struct EFI_FILE_PROTOCOL* This);

typedef struct EFI_FILE_PROTOCOL
{
  uint64_t              Revision;
  EFI_FILE_OPEN         Open;
  EFI_FILE_CLOSE        Close;
  EFI_FILE_DELETE       Delete;
  EFI_FILE_READ         Read;
  EFI_FILE_WRITE        Write;
  EFI_FILE_GET_POSITION GetPosition;
  EFI_FILE_SET_POSITION SetPosition;
  EFI_FILE_GET_INFO     GetInfo;
  EFI_FILE_SET_INFO     SetInfo;
  EFI_FILE_FLUSH        Flush;
  EFI_FILE_OPEN_EX      OpenEx;
  EFI_FILE_READ_EX      ReadEx;
  EFI_FILE_WRITE_EX     WriteEx;
  EFI_FILE_FLUSH_EX     FlushEx;
} EFI_FILE_PROTOCOL;

struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

typedef EFI_STATUS(EFI_API* EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME)(
    struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* This, EFI_FILE_PROTOCOL** Root);

typedef struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
{
  uint64_t                                    Revision;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME OpenVolume;
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

/**************************************************************
 * GIAO THỨC CỦA IMAGE ĐÃ TẢI
 **************************************************************/
typedef struct
{
  uint32_t   Revision;
  EFI_HANDLE ParentHandle;
  void*      SystemTable;

  // Vị trí nguồn của image trên đĩa
  EFI_HANDLE DeviceHandle;
  void*      FilePath; // EFI_DEVICE_PATH_PROTOCOL*
  void*      Reserved;

  // Các tùy chọn tải của image
  uint32_t LoadOptionsSize;
  void*    LoadOptions;

  // Vị trí bộ nhớ nơi image được nạp vào
  void*    ImageBase;
  uint64_t ImageSize;
  int      ImageCodeType; // EFI_MEMORY_TYPE
  int      ImageDataType; // EFI_MEMORY_TYPE
  void*    Unload;        // EFI_IMAGE_UNLOAD
} EFI_LOADED_IMAGE_PROTOCOL;

/**************************************************************
 * TIÊU ĐỀ BẢNG CỦA UEFI
 **************************************************************/
typedef struct
{
  uint64_t Signature;
  uint32_t Revision;
  uint32_t HeaderSize;
  uint32_t CRC32;
  uint32_t Reserved;
} EFI_TABLE_HEADER;

/**************************************************************
 * CẤU HÌNH BẢNG CỦA UEFI
 **************************************************************/
typedef struct
{
  EFI_GUID VendorGuid;
  void*    VendorTable;
} EFI_CONFIGURATION_TABLE;

/**************************************************************
 * GIAO THỨC XUẤT VĂN BẢN ĐƠN GIẢN
 **************************************************************/
struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef EFI_STATUS(EFI_API* EFI_TEXT_RESET)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    uint8_t                                  ExtendedVerification);
typedef EFI_STATUS(EFI_API* EFI_TEXT_STRING)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    uint16_t*                                String);
typedef EFI_STATUS(EFI_API* EFI_TEXT_TEST_STRING)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    uint16_t*                                String);
typedef EFI_STATUS(EFI_API* EFI_TEXT_QUERY_MODE)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    uint64_t                                 ModeNumber,
    uint64_t*                                Columns,
    uint64_t*                                Rows);
typedef EFI_STATUS(EFI_API* EFI_TEXT_SET_MODE)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    uint64_t                                 ModeNumber);
typedef EFI_STATUS(EFI_API* EFI_TEXT_SET_ATTRIBUTE)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    uint64_t                                 Attribute);
typedef EFI_STATUS(EFI_API* EFI_TEXT_CLEAR_SCREEN)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This);
typedef EFI_STATUS(EFI_API* EFI_TEXT_SET_CURSOR_POSITION)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    uint64_t                                 Column,
    uint64_t                                 Row);
typedef EFI_STATUS(EFI_API* EFI_TEXT_ENABLE_CURSOR)(
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    uint8_t                                  Visible);

typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL
{
  EFI_TEXT_RESET               Reset;
  EFI_TEXT_STRING              OutputString;
  EFI_TEXT_TEST_STRING         TestString;
  EFI_TEXT_QUERY_MODE          QueryMode;
  EFI_TEXT_SET_MODE            SetMode;
  EFI_TEXT_SET_ATTRIBUTE       SetAttribute;
  EFI_TEXT_CLEAR_SCREEN        ClearScreen;
  EFI_TEXT_SET_CURSOR_POSITION SetCursorPosition;
  EFI_TEXT_ENABLE_CURSOR       EnableCursor;
  void*                        Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

/**************************************************************
 * GIAO THỨC NHẬP VĂN BẢN ĐƠN GIẢN
 **************************************************************/

// Định nghĩa cấu trúc phím bấm trong UEFI
typedef struct
{
  uint16_t ScanCode;    // Mã quét bàn phím (ví dụ: mũi tên lên, xuống, F1, F2...)
  char16_t UnicodeChar; // Ký tự Unicode (ví dụ: 'a', 'b', '\r', '\n'...)
} EFI_INPUT_KEY;

// Khai báo trước (Forward declaration) để các hàm bên trong có thể tham chiếu
// ngược lại struct
struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

// Định nghĩa các con trỏ hàm theo chuẩn gọi của Microsoft (ms_abi)
typedef EFI_STATUS(EFI_API* EFI_INPUT_RESET)(
    struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL* This,
    bool                                    ExtendedVerification);

typedef EFI_STATUS(EFI_API* EFI_INPUT_READ_KEY)(
    struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL* This,
    EFI_INPUT_KEY*                          Key);

// Cấu trúc hoàn chỉnh của Simple Input Protocol
typedef struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL
{
  EFI_INPUT_RESET    Reset;
  EFI_INPUT_READ_KEY ReadKeyStroke;
  void*              WaitForKey; // Định nghĩa là void* nếu bạn chưa cần dùng sự kiện (Event) nâng cao
} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

/**************************************************************
 * GIAO THỨC XUẤT ĐỒ HỌA (GOP)
 **************************************************************/
typedef enum {
  PixelRedGreenBlueReserved8BitPerColor,
  PixelBlueGreenRedReserved8BitPerColor,
  PixelBitMask,
  PixelBltOnly,
  PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

typedef struct
{
  uint32_t RedMask;
  uint32_t GreenMask;
  uint32_t BlueMask;
  uint32_t ReservedMask;
} EFI_PIXEL_BITMASK;

typedef struct
{
  uint32_t                  Version;
  uint32_t                  HorizontalResolution;
  uint32_t                  VerticalResolution;
  EFI_GRAPHICS_PIXEL_FORMAT PixelFormat;
  EFI_PIXEL_BITMASK         PixelInformation;
  uint32_t                  PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct
{
  uint32_t                              MaxMode;
  uint32_t                              Mode;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info;
  uint64_t                              SizeOfInfo;
  uint64_t                              FrameBufferBase;
  uint64_t                              FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

struct _EFI_GRAPHICS_OUTPUT_PROTOCOL;
typedef EFI_STATUS(EFI_API* EFI_GRAPHICS_OUTPUT_QUERY_MODE)(
    struct _EFI_GRAPHICS_OUTPUT_PROTOCOL*  This,
    uint32_t                               ModeNumber,
    uint64_t*                              SizeOfInfo,
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION** Info);
typedef EFI_STATUS(EFI_API* EFI_GRAPHICS_OUTPUT_SET_MODE)(
    struct _EFI_GRAPHICS_OUTPUT_PROTOCOL* This,
    uint32_t                              ModeNumber);
typedef EFI_STATUS(EFI_API* EFI_GRAPHICS_OUTPUT_BLT)(
    struct _EFI_GRAPHICS_OUTPUT_PROTOCOL* This,
    void*                                 BltBuffer,
    uint32_t                              BltOperation,
    uint64_t                              SourceX,
    uint64_t                              SourceY,
    uint64_t                              DestinationX,
    uint64_t                              DestinationY,
    uint64_t                              Width,
    uint64_t                              Height,
    uint64_t                              Delta);

typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL
{
  EFI_GRAPHICS_OUTPUT_QUERY_MODE     QueryMode;
  EFI_GRAPHICS_OUTPUT_SET_MODE       SetMode;
  EFI_GRAPHICS_OUTPUT_BLT            Blt;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

/**************************************************************
 * CẤU TRÚC BỘ NHỚ
 **************************************************************/
typedef enum {
  EfiReservedMemoryType,
  EfiLoaderCode,
  EfiLoaderData,
  EfiBootServicesCode,
  EfiBootServicesData,
  EfiRuntimeServicesCode,
  EfiRuntimeServicesData,
  EfiConventionalMemory,
  EfiUnusableMemory,
  EfiACPIReclaimMemory,
  EfiACPIMemoryNVS,
  EfiMemoryMappedIO,
  EfiMemoryMappedIOPortSpace,
  EfiPalCode,
  EfiPersistentMemory,
  EfiMaxMemoryType
} EFI_MEMORY_TYPE;

typedef enum {
  AllocateAnyPages,
  AllocateMaxAddress,
  AllocateAddress,
  MaxAllocateType
} EFI_ALLOCATE_TYPE;

typedef struct
{
  uint32_t Type;
  uint64_t PhysicalStart;
  uint64_t VirtualStart;
  uint64_t NumberOfPages;
  uint64_t Attribute;
} EFI_MEMORY_DESCRIPTOR;

/**************************************************************
 * GIAO THỨC DỊCH VỤ KHỞI ĐỘNG
 **************************************************************/
typedef void (*EFI_EVENT_NOTIFY)(EFI_EVENT Event, void* Context);
typedef enum {
  TimerDefault,
  TimerPeriod,
  TimerRelative
} EFI_TIMER_DELAY;
typedef enum {
  AllHandles,
  ByRegisterNotify,
  ByProtocol
} EFI_INTERFACE_SEARCH_TYPE;

typedef struct
{
  EFI_HANDLE AgentHandle;
  EFI_HANDLE ControllerHandle;
  uint32_t   Attributes;
  uint32_t   OpenCount;
} EFI_OPEN_PROTOCOL_INFORMATION_ENTRY;

typedef EFI_TPL(EFI_API* EFI_RAISE_TPL)(EFI_TPL NewTpl);
typedef void(EFI_API* EFI_RESTORE_TPL)(EFI_TPL OldTpl);
typedef EFI_STATUS(EFI_API* EFI_ALLOCATE_PAGES)(
    EFI_ALLOCATE_TYPE Type,
    EFI_MEMORY_TYPE   MemoryType,
    uint64_t          Pages,
    uint64_t*         Memory);
typedef EFI_STATUS(EFI_API* EFI_FREE_PAGES)(
    uint64_t Memory,
    uint64_t Pages);
typedef EFI_STATUS(EFI_API* EFI_GET_MEMORY_MAP)(
    uint64_t*              MemoryMapSize,
    EFI_MEMORY_DESCRIPTOR* MemoryMap,
    uint64_t*              MapKey,
    uint64_t*              DescriptorSize,
    uint32_t*              DescriptorVersion);
typedef EFI_STATUS(EFI_API* EFI_ALLOCATE_POOL)(
    EFI_MEMORY_TYPE PoolType,
    uint64_t        Size,
    void**          Buffer);
typedef EFI_STATUS(EFI_API* EFI_FREE_POOL)(void* Buffer);
typedef EFI_STATUS(EFI_API* EFI_CREATE_EVENT)(
    uint32_t         Type,
    EFI_TPL          NotifyTpl,
    EFI_EVENT_NOTIFY NotifyFunction,
    void*            NotifyContext,
    EFI_EVENT*       Event);
typedef EFI_STATUS(EFI_API* EFI_SET_TIMER)(
    EFI_EVENT       Event,
    EFI_TIMER_DELAY Type,
    uint64_t        TriggerTime);
typedef EFI_STATUS(EFI_API* EFI_WAIT_FOR_EVENT)(
    uint64_t   NumberOfEvents,
    EFI_EVENT* Event,
    uint64_t*  Index);
typedef EFI_STATUS(EFI_API* EFI_SIGNAL_EVENT)(EFI_EVENT Event);
typedef EFI_STATUS(EFI_API* EFI_CLOSE_EVENT)(EFI_EVENT Event);
typedef EFI_STATUS(EFI_API* EFI_CHECK_EVENT)(EFI_EVENT Event);
typedef EFI_STATUS(EFI_API* EFI_INSTALL_PROTOCOL_INTERFACE)(
    EFI_HANDLE* Handle,
    EFI_GUID*   Protocol,
    uint32_t    InterfaceType,
    void*       Interface);
typedef EFI_STATUS(EFI_API* EFI_REINSTALL_PROTOCOL_INTERFACE)(
    EFI_HANDLE Handle,
    EFI_GUID*  Protocol,
    void*      OldInterface,
    void*      NewInterface);
typedef EFI_STATUS(EFI_API* EFI_UNINSTALL_PROTOCOL_INTERFACE)(
    EFI_HANDLE Handle,
    EFI_GUID*  Protocol,
    void*      Interface);
typedef EFI_STATUS(EFI_API* EFI_HANDLE_PROTOCOL)(
    EFI_HANDLE Handle,
    EFI_GUID*  Protocol,
    void**     Interface);
typedef void* VoidReservedPointer;
typedef EFI_STATUS(EFI_API* EFI_REGISTER_PROTOCOL_NOTIFY)(
    EFI_GUID* Protocol,
    EFI_EVENT Event,
    void**    Registration);
typedef EFI_STATUS(EFI_API* EFI_LOCATE_HANDLE)(
    EFI_INTERFACE_SEARCH_TYPE SearchType,
    EFI_GUID*                 Protocol,
    void*                     SearchKey,
    uint64_t*                 BufferSize,
    EFI_HANDLE*               Buffer);
typedef EFI_STATUS(EFI_API* EFI_LOCATE_DEVICE_PATH)(
    EFI_GUID*                  Protocol,
    EFI_DEVICE_PATH_PROTOCOL** DevicePath,
    EFI_HANDLE*                Device);
typedef EFI_STATUS(EFI_API* EFI_INSTALL_CONFIGURATION_TABLE)(
    EFI_GUID* Guid,
    void*     Table);
typedef EFI_STATUS(EFI_API* EFI_IMAGE_LOAD)(
    uint8_t                   BootPolicy,
    EFI_HANDLE                ParentImageHandle,
    EFI_DEVICE_PATH_PROTOCOL* DevicePath,
    void*                     SourceBuffer,
    uint64_t                  SourceSize,
    EFI_HANDLE*               ImageHandle);
typedef EFI_STATUS(EFI_API* EFI_IMAGE_START)(
    EFI_HANDLE ImageHandle,
    uint64_t*  ExitDataSize,
    uint16_t** ExitData);
typedef EFI_STATUS(EFI_API* EFI_EXIT)(
    EFI_HANDLE ImageHandle,
    EFI_STATUS ExitStatus,
    uint64_t   ExitDataSize,
    uint16_t*  WatchdogData);
typedef EFI_STATUS(EFI_API* EFI_IMAGE_UNLOAD)(EFI_HANDLE ImageHandle);
typedef EFI_STATUS(EFI_API* EFI_EXIT_BOOT_SERVICES)(
    EFI_HANDLE ImageHandle,
    uint64_t   MapKey);
typedef EFI_STATUS(EFI_API* EFI_GET_NEXT_MONOTONIC_COUNT)(uint64_t* Count);
typedef EFI_STATUS(EFI_API* EFI_STALL)(uint64_t Microseconds);
typedef EFI_STATUS(EFI_API* EFI_SET_WATCHDOG_TIMER)(
    uint64_t  Timeout,
    uint64_t  WatchdogCode,
    uint64_t  DataSize,
    uint16_t* WatchdogData);
typedef EFI_STATUS(EFI_API* EFI_CONNECT_CONTROLLER)(
    EFI_HANDLE                ControllerHandle,
    EFI_HANDLE*               DriverImageHandle,
    EFI_DEVICE_PATH_PROTOCOL* RemainingDevicePath,
    uint8_t                   Recursive);
typedef EFI_STATUS(EFI_API* EFI_DISCONNECT_CONTROLLER)(
    EFI_HANDLE ControllerHandle,
    EFI_HANDLE DriverImageHandle,
    EFI_HANDLE ChildHandle);
typedef EFI_STATUS(EFI_API* EFI_OPEN_PROTOCOL)(
    EFI_HANDLE Handle,
    EFI_GUID*  Protocol,
    void**     Interface,
    EFI_HANDLE AgentHandle,
    EFI_HANDLE ControllerHandle,
    uint32_t   Attributes);
typedef EFI_STATUS(EFI_API* EFI_CLOSE_PROTOCOL)(
    EFI_HANDLE Handle,
    EFI_GUID*  Protocol,
    EFI_HANDLE AgentHandle,
    EFI_HANDLE ControllerHandle);
typedef EFI_STATUS(EFI_API* EFI_OPEN_PROTOCOL_INFORMATION)(
    EFI_HANDLE                            Handle,
    EFI_GUID*                             Protocol,
    EFI_OPEN_PROTOCOL_INFORMATION_ENTRY** EntryBuffer,
    uint32_t*                             EntryCount);
typedef EFI_STATUS(EFI_API* EFI_PROTOCOLS_PER_HANDLE)(
    EFI_HANDLE  Handle,
    EFI_GUID*** ProtocolBuffer,
    uint32_t*   ProtocolBufferCount);
typedef EFI_STATUS(EFI_API* EFI_LOCATE_HANDLE_BUFFER)(
    EFI_INTERFACE_SEARCH_TYPE SearchType,
    EFI_GUID*                 Protocol,
    void*                     SearchKey,
    uint64_t*                 NoHandles,
    EFI_HANDLE**              Buffer);
typedef EFI_STATUS(EFI_API* EFI_LOCATE_PROTOCOL)(
    EFI_GUID* Protocol,
    void*     Registration,
    void**    Interface);
typedef EFI_STATUS(EFI_API* EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES)(
    EFI_HANDLE* Handle, ...);
typedef EFI_STATUS(EFI_API* EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES)(
    EFI_HANDLE Handle, ...);
typedef EFI_STATUS(EFI_API* EFI_CALCULATE_CRC32)(
    void*     Data,
    uint64_t  DataSize,
    uint32_t* Crc32);
typedef void(EFI_API* EFI_COPY_MEM)(
    void*    Destination,
    void*    Source,
    uint64_t Length);
typedef void(EFI_API* EFI_SET_MEM)(
    void*    Buffer,
    uint64_t Size,
    uint8_t  Value);
typedef EFI_STATUS(EFI_API* EFI_CREATE_EVENT_EX)(
    uint32_t         Type,
    EFI_TPL          NotifyTpl,
    EFI_EVENT_NOTIFY NotifyFunction,
    void*            NotifyContext,
    EFI_GUID*        EventGroup,
    EFI_EVENT*       Event);

typedef struct
{
  EFI_TABLE_HEADER                           Hdr;
  EFI_RAISE_TPL                              RaiseTPL;
  EFI_RESTORE_TPL                            RestoreTPL;
  EFI_ALLOCATE_PAGES                         AllocatePages;
  EFI_FREE_PAGES                             FreePages;
  EFI_GET_MEMORY_MAP                         GetMemoryMap;
  EFI_ALLOCATE_POOL                          AllocatePool;
  EFI_FREE_POOL                              FreePool;
  EFI_CREATE_EVENT                           CreateEvent;
  EFI_SET_TIMER                              SetTimer;
  EFI_WAIT_FOR_EVENT                         WaitForEvent;
  EFI_SIGNAL_EVENT                           SignalEvent;
  EFI_CLOSE_EVENT                            CloseEvent;
  EFI_CHECK_EVENT                            CheckEvent;
  EFI_INSTALL_PROTOCOL_INTERFACE             InstallProtocolInterface;
  EFI_REINSTALL_PROTOCOL_INTERFACE           ReinstallProtocolInterface;
  EFI_UNINSTALL_PROTOCOL_INTERFACE           UninstallProtocolInterface;
  EFI_HANDLE_PROTOCOL                        HandleProtocol;
  void*                                      VoidReserved;
  EFI_REGISTER_PROTOCOL_NOTIFY               RegisterProtocolNotify;
  EFI_LOCATE_HANDLE                          LocateHandle;
  EFI_LOCATE_DEVICE_PATH                     LocateDevicePath;
  EFI_INSTALL_CONFIGURATION_TABLE            InstallConfigurationTable;
  EFI_IMAGE_LOAD                             LoadImage;
  EFI_IMAGE_START                            StartImage;
  EFI_EXIT                                   Exit;
  EFI_IMAGE_UNLOAD                           UnloadImage;
  EFI_EXIT_BOOT_SERVICES                     ExitBootServices;
  EFI_GET_NEXT_MONOTONIC_COUNT               GetNextMonotonicCount;
  EFI_STALL                                  Stall;
  EFI_SET_WATCHDOG_TIMER                     SetWatchdogTimer;
  EFI_CONNECT_CONTROLLER                     ConnectController;
  EFI_DISCONNECT_CONTROLLER                  DisconnectController;
  EFI_OPEN_PROTOCOL                          OpenProtocol;
  EFI_CLOSE_PROTOCOL                         CloseProtocol;
  EFI_OPEN_PROTOCOL_INFORMATION              OpenProtocolInformation;
  EFI_PROTOCOLS_PER_HANDLE                   ProtocolsPerHandle;
  EFI_LOCATE_HANDLE_BUFFER                   LocateHandleBuffer;
  EFI_LOCATE_PROTOCOL                        LocateProtocol;
  EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES   InstallMultipleProtocolInterfaces;
  EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES UninstallMultipleProtocolInterfaces;
  EFI_CALCULATE_CRC32                        CalculateCrc32;
  EFI_COPY_MEM                               CopyMem;
  EFI_SET_MEM                                SetMem;
  EFI_CREATE_EVENT_EX                        CreateEventEx;
} EFI_BOOT_SERVICES;

/**************************************************************
 * BẢNG HỆ THỐNG
 **************************************************************/

typedef struct
{
  EFI_TABLE_HEADER                 Hdr;
  uint16_t*                        FirmwareVendor;
  uint32_t                         FirmwareRevision;
  EFI_HANDLE                       ConsoleInHandle;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL*  ConIn; // Giao thức nhập văn bản đơn giản
  EFI_HANDLE                       ConsoleOutHandle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConOut;
  EFI_HANDLE                       StandardErrorHandle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* StdErr;
  void*                            RuntimeServices; // EFI_RUNTIME_SERVICES
  EFI_BOOT_SERVICES*               BootServices;
  uint64_t                         NumberOfTableEntries;
  EFI_CONFIGURATION_TABLE*         ConfigurationTable; // Trỏ mảng cấu hình hệ thống
} EFI_SYSTEM_TABLE;

#endif // __SHARED__EFI_HPP