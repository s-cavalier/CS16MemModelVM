CXX = g++
CXXFLAGS = -Werror -Wall -Wextra -Wno-unused-parameter -g

mipstracer: main.o Utils.o
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f mipstracer ../mipstracer *.o branch.trace ../branch.trace