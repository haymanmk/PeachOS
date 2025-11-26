# PeachOS

[TOC]

## QuickStart

```bash
# assembling
as -o boot.o boot.S
# linking
ld -Ttext 0x7c00 --oformat binary -o boot.bin boot.o
```
> Note:
> The flag `-Ttext 0x7C00` tells linker to load `.text` section at memory address `0x7C00`. If not doing so, it will be prone into an error as shown below, which means the address of the instruction at offset `0xe` from the start of `.text` section will exceed 16 bits and need to be truncated, leading to this conflict.
>
> ```bash
> boot.o: in function `_start':
> (.text+0xe): relocation truncated to fit: R_X86_64_16 against `.text'+28
> ```
> A fun fact is that the same error occurs if the start of `.text` is set too large to fit instruction addresses within 16 bits.

Now, we can run the built binary file with QEMU by using the following command.

```bash
qemu-system-x86_64 -hda boot.bin

# -hda stands for hard disk.
```

## I/O Ports

I/O ports, a system hardware created by circuitry, are configured to communicate with peripheral devices. The available ports are as shown below:

| Port range    | Summary                                                      |
| ------------- | ------------------------------------------------------------ |
| 0x0000-0x001F | The first legacy [DMA controller](https://wiki.osdev.org/ISA_DMA), often used for transfers to floppies. |
| 0x0020-0x0021 | The first [Programmable Interrupt Controller](https://wiki.osdev.org/PIC) |
| 0x0022-0x0023 | Access to the Model-Specific Registers of Cyrix processors.  |
| 0x0040-0x0047 | The [PIT](https://wiki.osdev.org/PIT) (Programmable Interval Timer) |
| 0x0060-0x0064 | The ["8042" PS/2 Controller](https://wiki.osdev.org/"8042"_PS/2_Controller) or its predecessors, dealing with keyboards and mice. |
| 0x0070-0x0071 | The [CMOS](https://wiki.osdev.org/CMOS) and [RTC](https://wiki.osdev.org/RTC) registers |
| 0x0080-0x008F | The [DMA](https://wiki.osdev.org/DMA) (Page registers)       |
| 0x0092        | The location of the fast [A20](https://wiki.osdev.org/A20) gate register |
| 0x00A0-0x00A1 | The second [PIC](https://wiki.osdev.org/PIC)                 |
| 0x00C0-0x00DF | The second [DMA](https://wiki.osdev.org/DMA) controller, often used for soundblasters |
| 0x00E9        | Home of the [Port E9 Hack](https://wiki.osdev.org/Bochs). Used on some emulators to directly send text to the hosts' console. |
| 0x0170-0x0177 | The secondary [ATA](https://wiki.osdev.org/ATA) harddisk controller. |
| 0x01F0-0x01F7 | The primary [ATA](https://wiki.osdev.org/ATA) harddisk controller. |
| 0x0278-0x027A | Parallel port                                                |
| 0x02F8-0x02FF | Second [serial port](https://wiki.osdev.org/Serial_Ports)    |
| 0x03B0-0x03DF | The range used for the [IBM VGA](https://wiki.osdev.org/VGA_Hardware), its direct predecessors, as well as any modern video card in legacy mode. |
| 0x03F0-0x03F7 | [Floppy disk controller](https://wiki.osdev.org/FDC)         |
| 0x03F8-0x03FF | First [serial port](https://wiki.osdev.org/Serial_Ports)     |

## PIC - Programmable Interrupt Controller

PIC is a hardware components integrated into chipset (formerly, it is a chip mounted at motherboard). It is responsible for managing interrupt events from peripherals and communicating them to the CPU. e.g. signal CPU there is an interrupt occurs and provide its corresponding handler information by looking into vector table such idt (interrupt descriptor table).

There is a pitfall. The interrupt numbers ranging from 0 to 0x1F (0 to 31) are reserved for Intel CPU in protected mode. However, the interrupt numbering for PIC also starts from 0 to 7, which means these 8 interrupt numbers overlap with those reserved numbers of Intel CPU. A common solution to address these conflicts is to offset the starting point of PIC interrupt numbering, e.g. IRQ0->IRQ20, IRQ1->IRQ21. In order to modify its interrupt numbering, we can take the advantage of its programmable property by setting the interrupt vector address to an offset value such as 0x20.
