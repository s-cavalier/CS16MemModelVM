<<<<<<< HEAD
# ----------------------------------------------------------------------
# Compiler and “release” flags
# ----------------------------------------------------------------------
CXX       := g++
CXXFLAGS  := -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable
             
# (Remove the hard‐coded -DDEBUG here; we’ll only add it in debug mode.)
#
#CXXFLAGS += -DDEBUG
=======
# Compiler and flags
CXX := g++
CXXFLAGS := -O3 -Wall -Wextra -Werror -Wno-unused-variable
>>>>>>> parent of 6dc6812 (Added debug hooks for instructions)

# ----------------------------------------------------------------------
# Source directories and file lists
# ----------------------------------------------------------------------
SRC_DIRS  := machine machine/instructions loader
SRC       := $(wildcard main.cpp $(addsuffix /*.cpp,$(SRC_DIRS)))
OBJ       := $(SRC:.cpp=.o)

# ----------------------------------------------------------------------
# Final executable name
# ----------------------------------------------------------------------
TARGET    := spimulator

# ----------------------------------------------------------------------
# “all” (default) builds the optimized/release version
# ----------------------------------------------------------------------
.PHONY: all
all: CXXFLAGS += -O3
all: $(TARGET)

# Link object files into the final executable
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@

# Compile each .cpp → .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ----------------------------------------------------------------------
# “debug” target
# ----------------------------------------------------------------------
# Running “make debug” will:
#   1. append -DDEBUG (and any other debug flags)
#   2. build the same $(TARGET)
#
.PHONY: debug
debug: CXXFLAGS += -D DEBUG -g -O0
debug: $(TARGET)

# ----------------------------------------------------------------------
# Clean up build artifacts
# ----------------------------------------------------------------------
.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)
