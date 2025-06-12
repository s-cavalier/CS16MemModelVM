CC        := mips-linux-gnu-g++
AS        := mips-linux-gnu-as

ARCH_FLAGS := -march=mips32 -mfp32 -mno-dsp -mno-mips16 -mno-micromips -nostdlib -static -fno-pic -fno-pie -mno-abicalls -mno-gpopt -nostartfiles

LDFLAGS    := $(ARCH_FLAGS) -T linker.ld
COMMON_FLAGS := -Wall -Wextra
CFLAGS := $(ARCH_FLAGS) $(COMMON_FLAGS)

ASFLAGS := -march=mips32

SRCS := $(wildcard *.cpp)
ASM_SRCS := kerneltrap.asm

OBJS := $(SRCS:.cpp=.o) $(ASM_SRCS:.asm=.o)

TARGET := kernel.elf

.PHONY: all clean

all: $(TARGET)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -f $(OBJS) $(TARGET)
