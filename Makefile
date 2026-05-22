CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

OBJS   = main.o lexer.o parser.o interpreter.o compiler.o utils.o
TARGET = yoloscript

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) -lm

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
