# Usage: make FILE=yourfile.s

AS = mips-linux-gnu-as
LD = mips-linux-gnu-ld
OBJDUMP = mips-linux-gnu-objdump
OBJCOPY = mips-linux-gnu-objcopy

ASFLAGS = 
LDFLAGS = -Ttext 0x00400024 -Tdata 0x10008000 -nostdlib

# Derive names from the input file
BASE = $(basename $(FILE))
OBJ = $(BASE).o
OUT = $(BASE).elf

all: $(OUT)

$(OBJ): $(FILE)
	$(AS) $(ASFLAGS) -o $@ $<

$(OUT): $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $<

dump: $(OUT)
	$(OBJDUMP) -D -m mips -EL $< > $(OUT).dump

clean:
	rm -f *.o *.elf *.dump
