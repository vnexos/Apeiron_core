# =========================================================
# KỊCH BẢN XÂY DỰNG CHO BỘ NẠP KHỞI ĐỘNG
#
# Copyright (c) 2026 VNExos Inc.
# Bảo lưu mọi quyền.
#
# Được cấp phép theo Giấy phép Độc quyền.
# Xem tệp LICENSE tại thư mục `internal` để biết thêm chi
# tiết.
# =========================================================
export

include $(ROOT_DIR)/config/compile.mk

BOOT_DIR       := $(INTERNAL_DIR)/bootloader
BOOT_SRC_DIR   := $(BOOT_DIR)/src
BOOT_BUILD_DIR := $(INTERNAL_BUILD_DIR)/bootloader
# Tệp ELF trung gian (chỉ dùng cho riscv64 trước khi chuyển sang PE)
BOOT_ELF_SO    := $(BOOT_BUILD_DIR)/boot.elf
SRCS           := $(shell find $(BOOT_SRC_DIR) -type f \( -name "*.cpp" -o -name "*.s" \))
OBJS           := $(patsubst $(BOOT_SRC_DIR)/%.cpp,$(BOOT_BUILD_DIR)/%.o,$(filter %.cpp,$(SRCS))) \
                  $(patsubst $(BOOT_SRC_DIR)/%.s,$(BOOT_BUILD_DIR)/%.s.o,$(filter %.s,$(SRCS)))

BOOT_EFI_BIN   := $(EFI_BIN_DIR)/vnexos.efi
$(eval $(call COMPILE_TEMPLATE,\
	$(BOOT_SRC_DIR),\
	$(BOOT_BUILD_DIR),\
	$(BOOT_FLAGS),\
	$(BOOT_ASMFLAGS),\
	-I$(BOOT_SHARED_DIR)))

.PHONY: all clean
all: \
	$(BOOT_EFI_BIN)
	@echo -e "$(MSG_VNEXOS) Hoàn tất biên dịch $(COLOR_AQUA)Bộ nạp khởi động VNExos$(MODE_RESET)."

# Với riscv64, quy trình build EFI gồm 2 bước:
# 1. ld.lld + tập lệnh liên kết -> ELF thực thi (mã bắt đầu từ Địa chỉ ảo=0x1000)
# 2. tools/elf2efi_riscv64.py -> Ứng dụng EFI chuẩn PE32+ đúng đặc tả UEFI
#    (có section .reloc rỗng bắt buộc để UEFI loader không từ chối image)
$(BOOT_EFI_BIN): $(OBJS)
	@mkdir -p $(EFI_BIN_DIR) $(BOOT_BUILD_DIR)
ifeq ($(ARCH),riscv64)
	@echo -e "$(MSG_LD) Đang liên kết ELF trung gian: $(BOOT_ELF_SO)"
	@$(BOOT_LD) $(BOOT_LDFLAGS) -T$(BOOT_LINKER_SCRIPT) -o $(BOOT_ELF_SO) $^
	@echo -e "$(MSG_LD) Đang chuyển đổi ELF -> PE/UEFI: $@"
	@$(RISCV_ELF2EFI) $(BOOT_ELF_SO) $@
else
	@echo -e "$(MSG_LD) Đang liên kết thành tệp EFI khởi động: $@"
	@$(BOOT_LD) $(BOOT_LDFLAGS) -o $@ $^
endif
	@echo -e "$(MSG_LD) Đã liên kết thành công thành bộ nạp khởi động!"

clean:
	@rm -rf $(BOOT_BUILD_DIR)
	@echo -e "$(MSG_CLEAN) Bộ nạp khởi động đã được làm sạch!"
