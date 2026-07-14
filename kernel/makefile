# =========================================================
# KỊCH BẢN XÂY DỰNG CHO NHÂN HỆ ĐIỀU HÀNH
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

ARCHS          := x86_64 aarch64 riscv64
KERN_DIR       := $(INTERNAL_DIR)/kernel
KERN_BUILD_DIR := $(INTERNAL_BUILD_DIR)/kernel
SRCS           := $(shell find $(KERN_DIR) -type f \( -name "*.cpp" -o -name "*.s" \))
X86_64_OBJS    := $(patsubst $(KERN_DIR)/%.cpp,$(KERN_BUILD_DIR)/%.x86_64.o,$(filter %.cpp,$(SRCS))) \
                  $(patsubst $(KERN_DIR)/%.s,$(KERN_BUILD_DIR)/%.x86_64.s.o,$(filter %.s,$(SRCS)))
AARCH64_OBJS   := $(patsubst $(KERN_DIR)/%.cpp,$(KERN_BUILD_DIR)/%.aarch64.o,$(filter %.cpp,$(SRCS))) \
                  $(patsubst $(KERN_DIR)/%.s,$(KERN_BUILD_DIR)/%.aarch64.s.o,$(filter %.s,$(SRCS)))
RISCV64_OBJS   := $(patsubst $(KERN_DIR)/%.cpp,$(KERN_BUILD_DIR)/%.riscv64.o,$(filter %.cpp,$(SRCS))) \
                  $(patsubst $(KERN_DIR)/%.s,$(KERN_BUILD_DIR)/%.riscv64.s.o,$(filter %.s,$(SRCS)))
KERN_LLD       := ld.lld
KERN_LD_FLAGS  := -pie --oformat=binary -T $(KERN_LINKER_SCRIPT)

BINS           := $(patsubst %, $(KERN_BUILD_DIR)/%.bin, $(ARCHS))

$(eval $(call HYPER_COMPILE_TEMPLATE,\
	$(KERN_DIR),\
	$(KERN_BUILD_DIR),\
	$(KERNEL_FLAGS),\
	$(KERNEL_ASMFLAGS)))

.PHONY: all clean
all: \
	$(KERNEL_FILE)
	@echo "$(MSG_VNEXOS) Hoàn tất biên dịch $(COLOR_AQUA)Nhân lõi Bản Nguyên$(MODE_RESET)."

$(KERN_BUILD_DIR)/x86_64.bin: $(X86_64_OBJS)
	@mkdir -p $(dir $@)
	@echo "$(MSG_LD) Đang liên kết tệp nhị phân thô x86_64: $@"
	@$(KERN_LLD) -m elf_x86_64 $(KERN_LD_FLAGS) -Map=$(KERN_BUILD_DIR)/x86_64.map $(X86_64_OBJS) -o $@

$(KERN_BUILD_DIR)/aarch64.bin: $(AARCH64_OBJS)
	@mkdir -p $(dir $@)
	@echo "$(MSG_LD) Đang liên kết tệp nhị phân thô aarch64: $@"
	@$(KERN_LLD) -m aarch64elf $(KERN_LD_FLAGS) -Map=$(KERN_BUILD_DIR)/aarch64.map $(AARCH64_OBJS) -o $@

$(KERN_BUILD_DIR)/riscv64.bin: $(RISCV64_OBJS)
	@mkdir -p $(dir $@)
	@echo "$(MSG_LD) Đang liên kết tệp nhị phân thô riscv64: $@"
	@$(KERN_LLD) -m elf64lriscv $(KERN_LD_FLAGS) -Map=$(KERN_BUILD_DIR)/riscv64.map $(RISCV64_OBJS) -o $@

$(KERNEL_FILE): $(BINS)
	@mkdir -p $(dir $@)
	@echo "$(MSG_LD) Đang liên kết: $@"
	@echo "$(MSG_LD) Đã liên kết thành công nhân lõi!"

clean:
	@rm -rf $(KERN_BUILD_DIR)
	@echo "$(MSG_CLEAN) Nhân lõi đã được làm sạch!"
