.PHONY: all clean

OS_BIN := peachos.bin
BOOT_BIN := boot.bin
KERNEL_BIN := kernel.bin

BUILD_DIR := build
SRC_DIR := src

BOOTLOADER_SRC := $(SRC_DIR)/boot/boot.S
BOOTLOADER_OBJ := $(BUILD_DIR)/boot.o

ASM_SRCS := $(shell find $(SRC_DIR) -name '*.S' ! -path '$(SRC_DIR)/boot/*')
ASM_OBJS := $(patsubst $(SRC_DIR)/%.S,$(BUILD_DIR)/%.S.o,$(ASM_SRCS))
AS := $(PREFIX)/bin/$(TARGET)-as
ASFLAGS := -g -gdwarf-2

# Kernel C files
C_SRCS := $(shell find $(SRC_DIR) -name '*.c')
C_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRCS))
C_INCLUDES := $(SRC_DIR)
CC := $(PREFIX)/bin/$(TARGET)-gcc
CC_FLAGS := -I$(C_INCLUDES) -g -O0 -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall

LD := $(PREFIX)/bin/$(TARGET)-ld
LDFLAGS := --oformat=elf32-i386

OBJCOPY := $(PREFIX)/bin/$(TARGET)-objcopy
OBJDUMP := $(PREFIX)/bin/$(TARGET)-objdump

# Get all unique directories from C and assembly source files
C_DIRS := $(sort $(dir $(C_SRCS)))
ASM_DIRS := $(sort $(dir $(ASM_SRCS)))
BUILD_SUBDIRS := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(sort $(C_DIRS) $(ASM_DIRS)))

# Define the mount directory for copying files
MOUNT_DIR := /mnt/peachos

all: $(BUILD_DIR) $(BOOT_BIN) $(KERNEL_BIN)
	@echo "Combine binaries into a single bootable image..."
	rm -f $(BUILD_DIR)/$(OS_BIN)
	dd if=$(BUILD_DIR)/$(BOOT_BIN) >> $(BUILD_DIR)/$(OS_BIN)
	dd if=$(BUILD_DIR)/$(KERNEL_BIN) >> $(BUILD_DIR)/$(OS_BIN)
	dd if=/dev/zero bs=1048576 count=16 >> $(BUILD_DIR)/$(OS_BIN)

	# Copy files into output binary file
	sudo mkdir -p $(MOUNT_DIR)
	sudo mount -o loop $(BUILD_DIR)/$(OS_BIN) $(MOUNT_DIR)
	sudo cp ./hello.txt $(MOUNT_DIR)
	sudo cp ./config.yml $(MOUNT_DIR)
	sudo cp ./README.md $(MOUNT_DIR)
	sudo umount $(MOUNT_DIR)
	sudo rmdir $(MOUNT_DIR)

	@echo "Build complete: $(BUILD_DIR)/$(OS_BIN)"

$(BUILD_DIR): $(BUILD_SUBDIRS)
	@echo "Creating build directory..."
	mkdir -p $(BUILD_DIR)
	@echo "\n"

$(BUILD_SUBDIRS):
	mkdir -p $@

$(BOOT_BIN): $(BOOTLOADER_OBJ) | $(BUILD_DIR)
	@echo "Building $(BOOT_BIN)"
	$(LD) $(LDFLAGS) -Ttext 0x7C00 --oformat binary -o $(BUILD_DIR)/$(BOOT_BIN) $(BOOTLOADER_OBJ)
	@echo "\n"

$(BOOTLOADER_OBJ): $(BOOTLOADER_SRC) | $(BUILD_DIR)
	@echo "Assembling bootloader..."
	$(AS) $(ASFLAGS) -o $@ $<
	@echo "\n"

$(KERNEL_BIN): $(ASM_OBJS) $(C_OBJS) | $(BUILD_DIR)
	@echo "Building $(KERNEL_BIN)"
	$(LD) $(LDFLAGS) -T $(SRC_DIR)/linker.ld -Map=$(BUILD_DIR)/kernel.map -o $(BUILD_DIR)/kernel.elf $(ASM_OBJS) $(C_OBJS)
	$(OBJCOPY) -O binary $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/$(KERNEL_BIN)
	@echo "\n"

# Build rules for assembly source files
$(BUILD_DIR)/%.S.o: $(SRC_DIR)/%.S | $(BUILD_DIR)
	@echo "Assembling $@"
# Preprocess assembly source files first
	$(CC) -E -P -I$(C_INCLUDES) $< | $(AS) $(ASFLAGS) -I$(C_INCLUDES) -o $@ -
	@echo "\n"

# Build rules for C source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling $@"
	$(CC) $(CC_FLAGS) -c -o $@ $<
	@echo "\n"

clean:
	@echo "Cleaning up..."
	@if [ -d $(BUILD_DIR) ]; then rm -rf $(BUILD_DIR)/*; fi
	@echo "\n"
