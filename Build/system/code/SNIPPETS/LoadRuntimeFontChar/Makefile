CC      = mips64-gcc
LD      = mips64-ld
OBJCOPY = mips64-objcopy
OBJDUMP = mips64-objdump
SRCS    = code1.c

# code1's ram address
ADDRESS = 0x800D6590

# default compilation flags
CFLAGS = -G 0 -Os -I/opt/n64/mips64/include/z64hdr/include -I/opt/n64/mips64/include/z64hdr/oot_u10 -fno-toplevel-reorder --std=gnu99 -mtune=vr4300 -mabi=32 -mips3 -mno-check-zero-division -fno-builtin -mno-explicit-relocs -mno-memcpy -fno-reorder-blocks -ffast-math -fno-unsafe-math-optimizations -mno-check-zero-division -fno-optimize-sibling-calls

LDFLAGS = --emit-relocs -T
LDFLAGS = -L/opt/n64/mips64/include/z64hdr/common -L/opt/n64/mips64/include/z64hdr/oot_u10/ -T /opt/n64/mips64/include/z64hdr/common/z64hdr.ld --emit-relocs

default: code1.bin

code1.bin: $(SRCS)
	@echo -n "ENTRY_POINT = " > entry.ld
	@echo -n $(ADDRESS) >> entry.ld
	@echo -n ";" >> entry.ld
	@$(CC) -c code1.c $(CFLAGS)
	@$(LD) -o code1.elf code1.o $(LDFLAGS) $(LDFILE)
	@$(OBJCOPY) -R .MIPS.abiflags -O binary code1.elf code1.bin
	@$(OBJDUMP) -t code1.elf | grep code1

clean:
	rm -f *.bin *.elf *.o