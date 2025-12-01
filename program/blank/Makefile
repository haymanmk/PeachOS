.PHONY: all clean

TARGET_NAME := blank
TARGET_BIN := $(TARGET_NAME).bin
BUILD_DIR := build
# Get the directory of the current Makefile as SRC_DIR
# SRC_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SRC_DIR := src
C_INCLUDES := $(SRC_DIR)

AS := $(PREFIX)/bin/$(TARGET)-as
ASFLAGS := -g -gdwarf-2
LD := $(PREFIX)/bin/$(TARGET)-ld
LDFLAGS := -T $(SRC_DIR)/linker.ld --oformat=elf32-i386
CC := $(PREFIX)/bin/$(TARGET)-gcc
CC_FLAGS := -g -O0 -ffreestanding -nostdlib -fpic -I$(C_INCLUDES) -Wall
OBJCOPY := $(PREFIX)/bin/$(TARGET)-objcopy
OBJDUMP := $(PREFIX)/bin/$(TARGET)-objdump

ASM_SRCS := $(shell find $(SRC_DIR) -name '*.S')
ASM_OBJS := $(patsubst $(SRC_DIR)/%.S,$(BUILD_DIR)/%.S.o,$(ASM_SRCS))
C_SRCS := $(shell find $(SRC_DIR) -name '*.c')
C_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRCS))

ASM_DIRS := $(sort $(dir $(ASM_SRCS)))
C_DIRS := $(sort $(dir $(C_SRCS)))
BUILD_SUBDIRS := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(sort $(ASM_DIRS) $(C_DIRS)))

all: $(BUILD_DIR) $(TARGET_BIN)
	@echo "/********************************\n"
	@echo "Build complete: $(BUILD_DIR)/$(TARGET_BIN) \n"
	@echo "********************************/\n"

$(BUILD_DIR):
	@echo "# Creating build directories..."
	mkdir -p $(BUILD_SUBDIRS)
	@echo "\n"

$(TARGET_BIN): $(ASM_OBJS) $(C_OBJS)
	@echo "# Linking $@ ..."
	$(LD) $(LDFLAGS) -Map=$(BUILD_DIR)/$(TARGET_BIN).map -o $(BUILD_DIR)/$(TARGET_NAME).elf $^
	@echo "# Generating binary file..."
	$(OBJCOPY) -O binary $(BUILD_DIR)/$(TARGET_NAME).elf $(BUILD_DIR)/$(TARGET_BIN)
	@echo "\n"

# Build rules for assembly files
$(BUILD_DIR)/%.S.o: $(SRC_DIR)/%.S
	@echo "# Assembling $< ..."
	# Preprocess and assemble
	$(CC) -E -P -I$(C_INCLUDES) $< | $(AS) $(ASFLAGS) -I$(C_INCLUDES) -o $@ -
	@echo "\n"

# Build rules for C files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "# Compiling $< ..."
	$(CC) $(CC_FLAGS) -c -o $@ $<
	@echo "\n"

clean:
	@echo "# Cleaning build directories..."
	rm -rf $(BUILD_DIR)
	@echo "\n"