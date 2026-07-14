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
include $(ROOT_DIR)/config/linker.mk

BOOT_DIR       := $(INTERNAL_DIR)/bootloader
BOOT_SRC_DIR   := $(BOOT_DIR)/src
BOOT_INC_DIR   := $(BOOT_DIR)/inc
BOOT_BUILD_DIR := $(INTERNAL_BUILD_DIR)/bootloader
# Tệp ELF trung gian (chỉ dùng cho riscv64 trước khi chuyển sang PE)
BOOT_ELF_SO    := $(BOOT_BUILD_DIR)/boot.elf
SRCS           := $(shell find $(BOOT_SRC_DIR) -type f \( -name "*.cpp" -o -name "*.s" \))
X86_64_OBJS    := $(patsubst $(BOOT_SRC_DIR)/%.cpp,$(BOOT_BUILD_DIR)/%.x86_64.o,$(filter %.cpp,$(SRCS))) \
                  $(patsubst $(BOOT_SRC_DIR)/%.s,$(BOOT_BUILD_DIR)/%.x86_64.s.o,$(filter %.s,$(SRCS))) \
				  $(shell find $(BOOT_SHARED_DIR) -type f \( -name "*.x86_64.o" \))
AARCH64_OBJS   := $(patsubst $(BOOT_SRC_DIR)/%.cpp,$(BOOT_BUILD_DIR)/%.aarch64.o,$(filter %.cpp,$(SRCS))) \
				  $(patsubst $(BOOT_SRC_DIR)/%.s,$(BOOT_BUILD_DIR)/%.aarch64.s.o,$(filter %.s,$(SRCS))) \
				  $(shell find $(BOOT_SHARED_DIR) -type f \( -name "*.aarch64.o" \))
RISCV64_OBJS   := $(patsubst $(BOOT_SRC_DIR)/%.cpp,$(BOOT_BUILD_DIR)/%.riscv64.o,$(filter %.cpp,$(SRCS))) \
				  $(patsubst $(BOOT_SRC_DIR)/%.s,$(BOOT_BUILD_DIR)/%.riscv64.s.o,$(filter %.s,$(SRCS))) \
				  $(shell find $(BOOT_SHARED_DIR) -type f \( -name "*.riscv64.o" \))

BOOT_EFI_BINS  := $(EFI_BIN_DIR)/vnexosx64.efi \
				  $(EFI_BIN_DIR)/vnexosaa64.efi \
				  $(EFI_BIN_DIR)/vnexosriscv64.efi
$(eval $(call HYPER_BOOT_COMPILE_TEMPLATE,\
	$(BOOT_SRC_DIR),\
	$(BOOT_BUILD_DIR),\
	$(CXX_BASE_FLAGS),\
	$(BOOT_ASMFLAGS),\
	-I$(BOOT_SHARED_DIR) -D__EFI_ALLOWED -I$(BOOT_INC_DIR)))
$(eval $(call EFI_LINK_TEMPLATE,\
	$(EFI_BIN_DIR),\
	$(X86_64_OBJS),\
	$(AARCH64_OBJS),\
	$(RISCV64_OBJS),vnexos_main,\
	$(BOOT_LINKER_SCRIPT)))

.PHONY: all clean
all: \
	$(BOOT_EFI_BINS)
	@echo "$(MSG_VNEXOS) Hoàn tất biên dịch $(COLOR_AQUA)Bộ nạp khởi động VNExos$(MODE_RESET)."

clean:
	@rm -rf $(BOOT_BUILD_DIR)
	@echo "$(MSG_CLEAN) Bộ nạp khởi động đã được làm sạch!"
