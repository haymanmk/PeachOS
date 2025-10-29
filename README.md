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

