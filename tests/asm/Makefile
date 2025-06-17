# Usage: make FILE=yourfile.s

AS = mips-linux-gnu-as
LD = mips-linux-gnu-ld

ASFLAGS = 
LDFLAGS = -Ttext 0x00400024 -Tdata 0x10008000 -nostdlib

# Derive names from the input file
BASE = byte.asm
OBJ = $(BASE).o
OUT = $(BASE).elf

all: $(OUT)

$(OBJ): $(FILE)
	$(AS) $(ASFLAGS) -o $@ $<

$(OUT): $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $<

clean:
	rm -f *.o *.elf *.dump
