/**
 * Copyright (c) 2026 VNExos
 * Bảo lưu mọi quyền.
 *
 * Được cấp phép theo Giấy phép Độc quyền.
 * Xem tệp LICENSE tại thư mục `internal` để biết thêm chi
 * tiết.
 *
 * @file mm.hpp
 * @brief Tệp định nghĩa các hàm liên quan tới việc thao
 * tác trên bộ nhớ.
 */
#pragma once

#include <efi.hpp>

typedef enum : uint64_t {
  // PHẦN CỨNG
  PAGE_PRESENT       = (1 << 0),
  PAGE_READ          = (1 << 1),
  PAGE_WRITE         = (1 << 2),
  PAGE_EXEC          = (1 << 3),
  PAGE_USER          = (1 << 4),
  PAGE_GLOBAL        = (1 << 5), // Không đẩy TLB khi đổi tiến trình
  PAGE_NOCACHE       = (1 << 6), // RAM Uncached thông thường
  PAGE_MMIO          = (1 << 7), // Ngoại vi phần cứng (Device-nGnRnE / Strong Order)
  PAGE_WRITE_COMBINE = (1 << 8), // Tối ưu riêng cho Video Framebuffer
  PAGE_HUGE          = (1 << 9),
  // PHẦN MỀM
  PAGE_COW       = (1 << 10), // Copy-on-Write
  PAGE_DIRTY     = (1 << 11), // Đã bị ghi đè, cần sync ra đĩa
  PAGE_ACCESSED  = (1 << 12), // Đã từng được truy cập (phục vụ LRU)
  PAGE_ANONYMOUS = (1 << 13), // Bộ nhớ Heap/Stack (không có file)
  PAGE_SHARED    = (1 << 14), // Chia sẻ giữa nhiều process
  PAGE_SWAPPED   = (1 << 15), // Đã bị đẩy xuống đĩa
  PAGE_PINNED    = (1 << 16), // Cấm swap (DMA buffer)
  PAGE_GUARD     = (1 << 17), // Chặn tràn stack (bắn lỗi nếu chạm vào)
  // KHÁC
  PAGE_NONLEAF = (1 << 18), // Có phải là bảng dưới cùng không?
} PageFlags;

typedef struct
{
  uint64_t entries[512];
} __attribute__((packed)) PageTable;

bool initMemoryManagement(EFI_BOOT_SERVICES* BootServices, EFI_MEMORY_DESCRIPTOR** _map, uint64_t* _mapSize, uint64_t* _descriptorSize);
/*
 * 1. NGUYÊN TẮC CHUNG (ALL ARCHITECTURES):
 *  - Entry Not-Present (PAGE_PRESENT == 0):
 *      + Bắt buộc trả về đúng 0 tuyệt đối (zeroed).
 *      + Tuyệt đối không để sót bit physicalAddress, bit quyền hay bit mặc định
 *        (AF, SH, permissive flags) vào entry unmapped.
 *  - Căn chỉnh địa chỉ Huge Page (Leaf):
 *      + Trang 2MB: physicalAddress bắt buộc chia hết cho 2MB (21-bit 0 cuối).
 *      + Trang 1GB: physicalAddress bắt buộc chia hết cho 1GB (30-bit 0 cuối).
 *
 * 2. KIẾN TRÚC x86_64:
 *  - Non-Leaf (PML4E, PDPTE, PDE trỏ bảng con):
 *      + Bit 0 (P) = 1.
 *      + Bit 1 (R/W) = 1 & Bit 2 (U/S) = 1 (Permissive): Quyền x86 tính theo phép
 *        AND logic từ root đến lá; nếu để 0 ở non-leaf sẽ bóp nghẹt cả cây con.
 *      + Bit 7 (PS) = 0: Bắt buộc để 0, nếu bật CPU sẽ hiểu nhầm là Leaf Huge Page.
 *      + Bit 63 (XD/NX) = 0: Permissive (tránh cấm exec toàn bộ cây con qua phép OR).
 *      + Bit 8 (G): CPU bỏ qua ở non-leaf, không set.
 *  - Leaf (PTE 4KB, PDE 2MB, PDPTE 1GB):
 *      + Bit 7: Ở cấp cuối (4KB) là PAT (cấm set PAGE_HUGE). Ở PDE/PDPTE là bit PS.
 *      + Bit 63 (XD/NX): Bắt buộc kernel phải set IA32_EFER.NXE = 1 (MSR 0xC0000080),
 *        nếu không CPU sẽ quăng #GP/#PF do vi phạm Reserved Bit.
 *      + Write Protect: Kernel cần bật CR0.WP = 1 để Ring 0 tôn trọng cờ Read-Only.
 *
 * 3. KIẾN TRÚC AArch64 (ARMv8-A, 4KB Granule):
 *  - Non-Leaf (Table Descriptor ở Level 0, 1, 2):
 *      + Bit [1:0] = 0b11 (Table descriptor).
 *      + Không gán cờ lá (AP, UXN, PXN, AF, SH, AttrIndx) vào entry này.
 *      + Bit [62:59] (Hierarchical Attributes) = 0: Để NSTable/APTable/XNTable = 0
 *        để quyền hạn do leaf entry quyết định hoàn toàn.
 *  - Leaf (Page Descriptor Level 3, Block Descriptor Level 1, 2):
 *      + Bit [1:0]: Cấp 3 (4KB) = 0b11; Cấp 1 (1GB) & Cấp 2 (2MB) = 0b01 (Block).
 *      + Bit 10 (AF - Access Flag): Luôn set = 1 để tránh lỗi Access Flag Fault
 *        trên các core chưa bật phần cứng TCR_EL1.HA.
 *      + Bit [9:8] (SH): Set 0b11 (Inner Shareable) để bảo đảm cache coherency cho SMP.
 *      + Bit [54:53] (UXN & PXN):
 *          * Cấm exec hoàn toàn: UXN = 1, PXN = 1.
 *          * Mã User (EL0): PXN = 1 (Kernel cấm chạy mã user), UXN = 0.
 *          * Mã Kernel (EL1): UXN = 1 (User cấm chạy mã kernel), PXN = 0.
 *      + Bit [4:2] (AttrIndx): Phải map khớp với thanh ghi MAIR_EL1 lúc boot
 *        (Idx 0: Normal Cacheable RAM, Idx 1: Device-nGnRnE MMIO, Idx 2: Non-cacheable).
 *
 * 4. KIẾN TRÚC RISC-V 64 (Sv39 / Sv48):
 *  - Non-Leaf (Pointer Entry):
 *      + Bit [3:1] (X, W, R) = 0 0 0: Bắt buộc toàn bộ bằng 0 để định danh con trỏ.
 *      + Bit [9:4] (U, G, A, D) = 0 & Bit [62:61] (PBMT) = 0: Bắt buộc bằng 0
 *        theo đúng RISC-V Privileged Spec (Reserved-Must-Be-Zero). Set vào sẽ bị trap.
 *  - Leaf (Page 4KB, MegaPage 2MB, GigaPage 1GB):
 *      + Nhận diện: Bất kỳ entry nào có R=1, W=1 hoặc X=1.
 *      + Cấm tổ hợp W=1, R=0 (Reserved): Luôn bảo đảm W=1 kéo theo R=1.
 *      + Hỗ trợ Execute-Only: R=0, W=0, X=1 (hợp lệ).
 *      + Bit 6 (A) & Bit 7 (D): Luôn chủ động set A=1 và D=1 (khi có Write) để tránh
 *        CPU bẫy trap bắt OS cập nhật thủ công nếu thiếu Hardware A/D updating.
 *      + Ràng buộc PPN Superpage: MegaPage 2MB bắt buộc PPN[0] (bit [18:10]) = 0;
 *        GigaPage 1GB bắt buộc PPN[1:0] (bit [27:10]) = 0.
 *
 */
uint64_t makePte(uint64_t physicalAddress, uint64_t flags);

PageTable* allocateZeroPageTable();

void* allocatePages(uint64_t numberOfPages);
void  clearPages();
