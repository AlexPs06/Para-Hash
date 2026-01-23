# CC=g++
# CFLAGS= -march=armv8-a+crypto
# LIB= -Os 
# SOURCES= CPB_V2.cpp
# SOURCES2= ParaHash-V3.cpp
# all: 
# 	$(CC) -o CPB $(SOURCES) $(SOURCES2)  $(LIB) $(CFLAGS) 
# clean: 
# 	rm *.o  


# Compiler
CXX := clang++

# Target
TARGET := test

# Source files
SRCS := CPB_V2.cpp ParaHash-V3.cpp

# Header files (for dependency tracking)
HDRS := ParaHash-V3.h

# Compiler flags
CXXFLAGS := -O3 -std=c++17 -march=armv8.5-a+crypto \
            -Wall -Wextra -Wno-unused-parameter

# Linker flags (empty but kept for clarity)
LDFLAGS :=

# Default rule
all: $(TARGET)

# Link
$(TARGET): $(SRCS) $(HDRS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

# Clean
clean:
	rm -f $(TARGET)

.PHONY: all clean
