# Made this with help of AI cause I forgot how C++ works

# Compiler to use
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++11 -O2

# SDL2 flags (assumes sdl2-config is available)
SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LIBS := $(shell sdl2-config --libs)

# Name of the output executable
TARGET = chip8_emulator.out

# Source files
SOURCES = main.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Header files
HEADERS = chip8.cpp

# Default target
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET) $(SDL_LIBS)

# Rule to compile source files to object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) -c $< -o $@

# Clean target for removing compiled files
clean:
	rm -f $(TARGET) $(OBJECTS)

# Phony targets
.PHONY: all clean