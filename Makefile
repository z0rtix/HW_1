CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Werror -O2 -g -fsanitize=address

TARGET = lab1
SRCS = main.cpp benchmark.cpp
HEADERS = UnqPtr.hpp ShrdPtr.hpp DynamicArray.hpp tests.hpp

all: $(TARGET)

$(TARGET): $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -rf $(TARGET) *.dSYM *.o