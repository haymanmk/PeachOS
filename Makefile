.PHONY: all clean

TARGET := boot.bin
OBJS := boot.o
AS := as
ASFLAGS := -g -odwarf-2
LD := ld
LDFLAGS := -Ttext 0x7C00 --oformat binary

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Building $(TARGET)"
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS)

$(OBJS): %.o: %.S
	@echo "Assembling $<"
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)
