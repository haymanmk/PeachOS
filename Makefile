.PHONY: all clean

TARGET := boot.bin
OBJS := boot.o
AS := as
ASFLAGS :=
LD := ld
LDFLAGS := -Ttext 0x7C00 --oformat binary

all: $(TARGET)
	@echo "Add another sector to store additional message."
	dd if=message.txt >> $(TARGET)
	# Pad the entire file to multiple of 512 bytes
	dd if=/dev/zero bs=512 count=1 >> $(TARGET)

$(TARGET): $(OBJS)
	@echo "Building $(TARGET)"
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS)

$(OBJS): %.o: %.S
	@echo "Assembling $<"
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)
