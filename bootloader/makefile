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
BOOT_BUILD_DIR := $(INTERNAL_BUILD_DIR)/bootloader
SRCS           := $(shell find $(BOOT_DIR) -type f \( -name "*.cpp" -o -name "*.s" \))
OBJS           := $(patsubst $(BOOT_DIR)/%.cpp,$(BOOT_BUILD_DIR)/%.o,$(filter %.cpp,$(SRCS))) \
                  $(patsubst $(BOOT_DIR)/%.s,$(BOOT_BUILD_DIR)/%.s.o,$(filter %.s,$(SRCS)))
$(eval $(call COMPILE_TEMPLATE,\
	$(BOOT_DIR),\
	$(BOOT_BUILD_DIR),\
	$(BOOT_FLAGS),\
	$(BOOT_ASMFLAGS),\
	-I$(SHARED_DIR)))

.PHONY: all clean
all: \
	$(EFI_BIN)
	@echo -e "$(MSG_VNEXOS) Hoàn tất biên dịch $(COLOR_AQUA)Bộ nạp khởi động VNExos$(MODE_RESET)."

$(EFI_BIN): \
	$(OBJS)
	@mkdir -p $(EFI_BIN_DIR)
	@echo -e "$(MSG_LD) Đang liên kết thành tệp EFI khởi động: $@"
	@$(BOOT_LD) $(BOOT_LDFLAGS) -o $@ $^
	@echo -e "$(MSG_LD) Đã liên kết thành công thành bộ nạp khởi động!"

clean:
	@rm -rf $(KERN_BUILD_DIR)
	@echo -e "$(MSG_CLEAN) Bộ nạp khởi động đã được làm sạch!"
