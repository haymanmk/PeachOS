.PHONY: all clean

TARGET_NAME := libstdlib
TARGET_BIN := $(TARGET_NAME).a
BUILD_DIR := build
SRC_DIR := src
C_INCLUDES := $(SRC_DIR)
KERNEL_LIB_DIR := ../../src/
C_INCLUDES += $(KERNEL_LIB_DIR)
C_INCLUDES_FLAGS := $(addprefix -I,$(C_INCLUDES))

AS := $(PREFIX)/bin/$(TARGET)-as
ASFLAGS := -g -gdwarf-2
LD := $(PREFIX)/bin/$(TARGET)-ld
LDFLAGS := -T $(SRC_DIR)/linker.ld --oformat=elf32-i386 -relocatable
CC := $(PREFIX)/bin/$(TARGET)-gcc
CC_FLAGS := $(C_INCLUDES_FLAGS) -g -O0 -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall
OBJCOPY := $(PREFIX)/bin/$(TARGET)-objcopy
OBJDUMP := $(PREFIX)/bin/$(TARGET)-objdump

ASM_SRCS := $(shell find $(SRC_DIR) -name '*.S')
ASM_OBJS := $(patsubst $(SRC_DIR)/%.S,$(BUILD_DIR)/%.S.o,$(ASM_SRCS))
C_SRCS := $(shell find $(SRC_DIR) -name '*.c')
C_OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRCS))

ASM_DIRS := $(sort $(dir $(ASM_SRCS)))
C_DIRS := $(sort $(dir $(C_SRCS)))
BUILD_SUBDIRS := $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(sort $(ASM_DIRS) $(C_DIRS)))

all: clean ${BUILD_DIR} $(TARGET_BIN)
	@echo "/********************************\n"
	@echo "Build complete: $(BUILD_DIR)/$(TARGET_BIN) \n"
	@echo "********************************/\n"

$(BUILD_DIR):
	@echo "\n# Creating build directories..."
	mkdir -p $(BUILD_SUBDIRS)

$(TARGET_BIN): $(ASM_OBJS) $(C_OBJS)
	@echo "\nLinking $@ ..."
	$(LD) $(LDFLAGS) -Map=$(BUILD_DIR)/$(TARGET_NAME).map -o $(BUILD_DIR)/$(TARGET_NAME).elf $^
	@echo "# Generating archive file..."
	ar rcs $(BUILD_DIR)/$(TARGET_BIN) $(ASM_OBJS) $(C_OBJS)

# Build rules for assembly files
$(BUILD_DIR)/%.S.o: $(SRC_DIR)/%.S
	@echo "\nAssembling $< ..."
	# Preprocess and assemble
	$(CC) -E -P $(C_INCLUDES_FLAGS) $< | $(AS) $(ASFLAGS) $(C_INCLUDES_FLAGS) -o $@ -

# Build rules for C files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "\nCompiling $< ..."
	$(CC) $(CC_FLAGS) -c -o $@ $<
	
clean:
	@echo "\n# Cleaning build directories..."
	rm -rf $(BUILD_DIR)