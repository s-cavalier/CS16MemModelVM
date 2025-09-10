#————————————————————————————————————————————————————————————
# Cross‐compiler settings
#————————————————————————————————————————————————————————————

CC        := mips-linux-gnu-g++

ARCH_FLAGS := -march=mips32 -mfp32 -mno-dsp -mno-mips16 -mno-micromips -nostdlib -ffreestanding -fno-rtti -fno-exceptions -static -fno-pic -fno-pie -mno-abicalls -mno-gpopt -nostartfiles
LDFLAGS    := $(ARCH_FLAGS) -T linker.ld
COMMON_FLAGS := -Wall -Wextra
CFLAGS := $(ARCH_FLAGS) $(COMMON_FLAGS)

#————————————————————————————————————————————————————————————
# Source discovery
#————————————————————————————————————————————————————————————

# Find .cpp files in current directory and ../ministl
SRCS := $(wildcard *.cpp) $(wildcard ../ministl/*.cpp)

# Convert each source file to a corresponding object file in the build dir
OBJS := $(SRCS:.cpp=.o)

# Final output filename
TARGET := calculator.elf

#————————————————————————————————————————————————————————————
# Make rules
#————————————————————————————————————————————————————————————

.PHONY: all clean

all: $(TARGET)

# Compile rule
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Link rule
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -f $(OBJS) $(TARGET)
