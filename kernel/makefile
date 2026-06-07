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

KERN_DIR       := $(INTERNAL_DIR)/kernel
KERN_BUILD_DIR := $(INTERNAL_BUILD_DIR)/kernel
SRCS           := $(shell find $(KERN_DIR) -type f \( -name "*.cpp" -o -name "*.s" \))
OBJS           := $(patsubst $(KERN_DIR)/%.cpp,$(KERN_BUILD_DIR)/%.o,$(filter %.cpp,$(SRCS))) \
                  $(patsubst $(KERN_DIR)/%.s,$(KERN_BUILD_DIR)/%.s.o,$(filter %.s,$(SRCS)))
$(eval $(call COMPILE_TEMPLATE,\
	$(KERN_DIR),\
	$(KERN_BUILD_DIR),\
	$(KERNEL_FLAGS),\
	$(KERNEL_ASMFLAGS),\
	-I$(SHARED_DIR)))

.PHONY: all clean
all: \
	$(KERNEL_FILE)
	@echo -e "$(MSG_VNEXOS) Hoàn tất biên dịch $(COLOR_AQUA)Nhân lõi Bản Nguyên$(MODE_RESET)."

$(KERNEL_FILE): $(OBJS)
	@mkdir -p $(dir $@)
	@echo -e "$(MSG_LD) Đang liên kết: $@"
	@$(KERN_LD) -T$(KERN_LINKER_SCRIPT) $(KERN_LDFLAGS) -o $@ $^
	@echo -e "$(MSG_LD) Đã liên kết thành công nhân lõi!"

clean:
	@rm -rf $(KERN_BUILD_DIR)
	@echo -e "$(MSG_CLEAN) Nhân lõi đã được làm sạch!"
