ifeq ($(OS),Windows_NT)
	MKDIR := wsl mkdir -p
else
	MKDIR := mkdir -p
endif

CXX := g++
CXXFLAGS := -Iinclude -Wall -MMD -MP -std=c++20 -g

ifeq ($(OS),Windows_NT)
	LIBFLAGS := -lws2_32
else
	LIBFLAGS := 
endif

SRC_DIR := src
BUILD_DIR := build
INC_DIR := include

ifeq ($(OS),Windows_NT)
	SRC_FILES := $(shell wsl find $(SRC_DIR) -name '*.cpp')
	INC_FILES := $(shell wsl find $(INC_DIR) -name '*.hpp')
else
	SRC_FILES := $(shell find $(SRC_DIR) -name '*.cpp')
	INC_FILES := $(shell find $(INC_DIR) -name '*.hpp')
endif

OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))
DEP_FILES := $(OBJ_FILES:.o=.d)

ifeq ($(OS),Windows_NT)
	TARGET := server.exe
else
	TARGET := server
endif

all: $(TARGET)

# Executable
$(TARGET): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBFLAGS)

# Object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(MKDIR) $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEP_FILES)

# Misc.
.PHONY: clean run all

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

run: $(TARGET)
	./$(TARGET)