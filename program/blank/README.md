# The Order of Sections in Linker Script

The order of sections in the linker script effects the first instruction that processor fetches for you. 

For instance, we load the binary file at virtual memory address `0x400000` at runtime. Processor does not understand if the first byte in the binary is an instruction or not. It just fetches them byte by byte, decode, and execute for us. So, if we put data sections (such as `.data` and `.bss`) at the very beginning of this binary file, processor will decode it as wrong instructions leading to unexpected results. According to this, it is better to place code sections (such as `.text` and `.asm`) before data sections and ensure that the object which contains the entry point (can be specified by the directive `ENTRY(_start)`) is arranged at the start of this binary.

The following capture shows part of the linker map. The first available byte the processor reads into memory is related to data instead of the executable code which is supposed to be read first. This is the result of incorrect order of sections in linker script.

```shell
Memory Configuration

Name             Origin             Length             Attributes
*default*        0x00000000         0xffffffff

Linker script and memory map

                0x00400000                        . = 0x400000

.text           0x00400000        0x0
 *(.text)
 .text          0x00400000        0x0 build/blank.S.o

.iplt           0x00400000        0x0
 .iplt          0x00400000        0x0 build/blank.S.o

.rodata
 *(.rodata)

.data           0x00400000       0x17
 *(.data)
 .data          0x00400000       0x17 build/blank.S.o

.got            0x00400018        0x0
 .got           0x00400018        0x0 build/blank.S.o

.got.plt        0x00400018        0x0
 .got.plt       0x00400018        0x0 build/blank.S.o

.igot.plt       0x00400018        0x0
 .igot.plt      0x00400018        0x0 build/blank.S.o

.bss            0x00401000        0x0
 *(COMMON)
 *(.bss)
 .bss           0x00401000        0x0 build/blank.S.o

.asm            0x00401000       0x11
 *(.asm)
 .asm           0x00401000       0x11 build/blank.S.o
                0x00401000                _start
LOAD build/blank.S.o
```

