CXX = g++
CXXFLAGS = -g

spimulator: main.o Hardware.o Instruction.o
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f *.o spimulator