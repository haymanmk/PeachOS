.PHONY: all clean

OS_BIN := peachos.bin
BOOT_BIN := boot.bin
KERNEL_BIN := kernel.bin

BUILD_DIR := build
SRC_DIR := src

BOOTLOADER_SRC := $(SRC_DIR)/boot/boot.S
BOOTLOADER_OBJ := $(BUILD_DIR)/boot.o

ASM_SRCS := $(wildcard $(SRC_DIR)/*.S)
ASM_OBJS := $(patsubst $(SRC_DIR)/%.S,$(BUILD_DIR)/%.S.o,$(wildcard $(SRC_DIR)/*.S))
AS := $(PREFIX)/bin/$(TARGET)-as
ASFLAGS := -g -gdwarf-2

# Kernel C files
C_SRCS := $(wildcard $(SRC_DIR)/*.c)
C_SRCS += $(wildcard $(SRC_DIR)/utils/*.c)
C_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRCS))
C_INCLUDES := $(SRC_DIR)
CC := $(PREFIX)/bin/$(TARGET)-gcc
CC_FLAGS := -I$(C_INCLUDES) -g -O0 -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall

LD := $(PREFIX)/bin/$(TARGET)-ld
LDFLAGS := --oformat=elf32-i386

OBJCOPY := $(PREFIX)/bin/$(TARGET)-objcopy
OBJDUMP := $(PREFIX)/bin/$(TARGET)-objdump

# Get all unique directories from source files
C_DIRS := $(sort $(dir $(C_SRCS)))
BUILD_SUBDIRS := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(C_DIRS))

all: $(BUILD_DIR) $(BOOT_BIN) $(KERNEL_BIN)
	@echo "Combine binaries into a single bootable image..."
	rm -f $(BUILD_DIR)/$(OS_BIN)
	dd if=$(BUILD_DIR)/$(BOOT_BIN) >> $(BUILD_DIR)/$(OS_BIN)
	dd if=$(BUILD_DIR)/$(KERNEL_BIN) >> $(BUILD_DIR)/$(OS_BIN)
	dd if=/dev/zero bs=512 count=100 >> $(BUILD_DIR)/$(OS_BIN)
	@echo "Build complete: $(BUILD_DIR)/$(OS_BIN)"

$(BUILD_DIR): $(BUILD_SUBDIRS)
	@echo "Creating build directory..."
	mkdir -p $(BUILD_DIR)

$(BUILD_SUBDIRS):
	mkdir -p $@

$(BOOT_BIN): $(BOOTLOADER_OBJ) | $(BUILD_DIR)
	@echo "Building $(BOOT_BIN)"
	$(LD) $(LDFLAGS) -Ttext 0x7C00 --oformat binary -o $(BUILD_DIR)/$(BOOT_BIN) $(BOOTLOADER_OBJ)

$(BOOTLOADER_OBJ): $(BOOTLOADER_SRC) | $(BUILD_DIR)
	@echo "Assembling bootloader..."
	$(AS) $(ASFLAGS) -o $@ $<

$(KERNEL_BIN): $(ASM_OBJS) $(C_OBJS) | $(BUILD_DIR)
	@echo "Building $(KERNEL_BIN)"
	$(LD) $(LDFLAGS) -T $(SRC_DIR)/linker.ld -Map=$(BUILD_DIR)/kernel.map -o $(BUILD_DIR)/kernel.elf $(ASM_OBJS) $(C_OBJS)
	$(OBJCOPY) -O binary $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/$(KERNEL_BIN)

$(ASM_OBJS): $(ASM_SRCS) | $(BUILD_DIR)
	@echo "Assembling $@"
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling $@"
	$(CC) $(CC_FLAGS) -c -o $@ $<

clean:
	@echo "Cleaning up..."
	@if [ -d $(BUILD_DIR) ]; then rm -rf $(BUILD_DIR)/*; fi
