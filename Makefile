CXX = g++
CXXFLAGS = -O3
INSTRUCTIONS = IInstruction.o RInstruction.o SpecialInstruction.o

spimulator: main.o Hardware.o $(INSTRUCTIONS) Loader.o
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f *.o spimulator