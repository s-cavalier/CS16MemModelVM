CXX       := g++
CXXFLAGS  := -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable
LDFLAGS   := -lssl -lcrypto

SRC_DIRS  := machine machine/instructions loader network
SRC       := $(wildcard main.cpp $(addsuffix /*.cpp,$(SRC_DIRS)))
OBJ       := $(SRC:.cpp=.o)

TARGET    := bobbyOS

.PHONY: all
all: CXXFLAGS += -O3
all: $(TARGET)

# Link object files into the final executable
$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

# Compile each .cpp -> .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: debug
debug: CXXFLAGS += -D DEBUG -g -O0
debug: $(TARGET)

.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)
