# Compiler and flags
CXX := g++
CXXFLAGS := -g -Wall -Wextra -Werror -Wno-unused-variable

# Directories
SRC_DIRS := machine
SRC := $(wildcard main.cpp $(addsuffix /*.cpp, $(SRC_DIRS)))
OBJ := $(SRC:.cpp=.o)

# Target executable
TARGET := spimulator

all: $(TARGET)

# Link object files into final executable
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@

# Compile each .cpp into .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
