.PHONY: all clean

OS_BIN := peachos.bin
BOOT_BIN := boot.bin
KERNEL_BIN := kernel.bin
BUILD_DIR := build
SRC_DIR := src
BOOTLOADER_SRC := $(SRC_DIR)/boot/boot.S
BOOTLOADER_OBJ := $(BUILD_DIR)/boot.o
ASM_SRCS := $(wildcard $(SRC_DIR)/*.S)
ASM_OBJS := $(patsubst $(SRC_DIR)/%.S,$(BUILD_DIR)/%.o,$(wildcard $(SRC_DIR)/*.S))
AS := as
ASFLAGS := -g -gdwarf-2
LD := ld
LDFLAGS :=

all: $(BUILD_DIR) $(BOOT_BIN) $(KERNEL_BIN)
	@echo "Combine binaries into a single bootable image..."
	rm -f $(BUILD_DIR)/$(OS_BIN)
	dd if=$(BUILD_DIR)/$(BOOT_BIN) >> $(BUILD_DIR)/$(OS_BIN)
	dd if=$(BUILD_DIR)/$(KERNEL_BIN) >> $(BUILD_DIR)/$(OS_BIN)
	dd if=/dev/zero bs=512 count=100 >> $(BUILD_DIR)/$(OS_BIN)
	@echo "Build complete: $(BUILD_DIR)/$(OS_BIN)"

$(BUILD_DIR):
	@echo "Creating build directory..."
	mkdir -p $(BUILD_DIR)

$(BOOT_BIN): $(BOOTLOADER_OBJ) | $(BUILD_DIR)
	@echo "Building $(BOOT_BIN)"
	$(LD) $(LDFLAGS) -Ttext 0x7C00 --oformat binary -o $(BUILD_DIR)/$(BOOT_BIN) $(BOOTLOADER_OBJ)

$(BOOTLOADER_OBJ): $(BOOTLOADER_SRC) | $(BUILD_DIR)
	@echo "Assembling bootloader..."
	$(AS) $(ASFLAGS) -o $@ $<

$(KERNEL_BIN): $(ASM_OBJS) | $(BUILD_DIR)
	@echo "Building $(KERNEL_BIN)"
	$(LD) $(LDFLAGS) -T $(SRC_DIR)/linker.ld --oformat binary -o $(BUILD_DIR)/$(KERNEL_BIN) $(ASM_OBJS)

$(ASM_OBJS): $(ASM_SRCS) | $(BUILD_DIR)
	@echo "Assembling $@"
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	@echo "Cleaning up..."
	@if [ -d $(BUILD_DIR) ]; then rm -rf $(BUILD_DIR)/*; fi
