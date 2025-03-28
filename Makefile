# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude -Iexternal -Iexternal/stb -I/usr/include/ws2811 -I/usr/include/libcamera -O3 -march=native -ffast-math

LDFLAGS = -L/usr/lib -lws2811 -lcamera -lcamera-base

# Directories
SRC_DIR = src
BUILD_DIR = build

# Source and Object files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

# Output executable
TARGET = $(BUILD_DIR)/main.exe

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(OBJS) $(LDFLAGS) -o $(TARGET)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(BUILD_DIR)