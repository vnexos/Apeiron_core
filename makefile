# =========================================================
# KỊCH BẢN XÂY DỰNG CHO LÕI HỆ ĐIỀU HÀNH
#
# Copyright (c) 2026 VNExos Inc.
# Bảo lưu mọi quyền.
#
# Được cấp phép theo Giấy phép Độc quyền.
# Xem tệp LICENSE tại thư mục `internal` để biết thêm chi
# tiết.
# =========================================================
export

INTERNAL_SUBDIRS   := bootloader kernel
INTERNAL_DIR       := $(ROOT_DIR)/internal
INTERNAL_BUILD_DIR := $(BUILD_DIR)/internal
SHARED_DIR         := $(INTERNAL_DIR)/shared
LINKER_SCRIPT      := $(INTERNAL_DIR)/linker_$(ARCH).ld

.PHONY: all clean $(INTERNAL_SUBDIRS)
all: \
	$(INTERNAL_SUBDIRS)
	@echo -e "$(MSG_VNEXOS) Đã xây dựng xong chương trình lõi!"

$(INTERNAL_SUBDIRS):
	@$(MAKE) -C $@

clean:
	@for dir in $(INTERNAL_SUBDIRS); do $(MAKE) -C $$dir clean; done
	@rm -rf $(INTERNAL_BUILD_DIR)
	@echo -e "$(MSG_CLEAN) Lõi đã được làm sạch sâu!"