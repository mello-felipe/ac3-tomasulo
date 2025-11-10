CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/estado.cpp $(SRC_DIR)/parser.cpp $(SRC_DIR)/utils.cpp

OBJECTS = $(BUILD_DIR)/main.o $(BUILD_DIR)/estado.o $(BUILD_DIR)/parser.o $(BUILD_DIR)/utils.o

TARGET = simulator

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)
	@echo "Build complete: $(TARGET)"

$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp $(INCLUDE_DIR)/parser.hpp $(INCLUDE_DIR)/estado.hpp $(INCLUDE_DIR)/types.hpp
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/main.cpp -o $(BUILD_DIR)/main.o

$(BUILD_DIR)/estado.o: $(SRC_DIR)/estado.cpp $(INCLUDE_DIR)/estado.hpp $(INCLUDE_DIR)/types.hpp
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/estado.cpp -o $(BUILD_DIR)/estado.o

$(BUILD_DIR)/parser.o: $(SRC_DIR)/parser.cpp $(INCLUDE_DIR)/parser.hpp $(INCLUDE_DIR)/types.hpp $(INCLUDE_DIR)/utils.hpp
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/parser.cpp -o $(BUILD_DIR)/parser.o

$(BUILD_DIR)/utils.o: $(SRC_DIR)/utils.cpp $(INCLUDE_DIR)/utils.hpp
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/utils.cpp -o $(BUILD_DIR)/utils.o

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Clean complete"

rebuild: clean all

run: $(TARGET)
	./$(TARGET) input.txt

.PHONY: all clean rebuild run