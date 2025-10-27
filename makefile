ifeq ($(OS),Windows_NT)
	MKDIR := wsl mkdir -p
else
	MKDIR := mkdir -p
endif

CXX := g++
CXXFLAGS := -Iinclude -Wall

ifeq ($(OS),Windows_NT)
	LIBFLAGS := -lws2_32
else
	LIBFLAGS := 
endif

SRC_DIR := src
OBJ_DIR := obj

ifeq ($(OS),Windows_NT)
	SRC_FILES := $(shell wsl find $(SRC_DIR) -name '*.cpp')
else
	SRC_FILES := $(shell find $(SRC_DIR) -name '*.cpp')
endif

OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))

ifeq ($(OS),Windows_NT)
	TARGET := server.exe
else
	TARGET := server
endif

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(MKDIR) $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

run: $(TARGET)
	./$(TARGET)