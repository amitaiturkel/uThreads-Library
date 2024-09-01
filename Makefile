# Define compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I. -Itests
AR = ar
ARFLAGS = rcs
BUILD_DIR = build
LIB_DIR = $(BUILD_DIR)
LIBRARY = libuthreads.a

# Define test source files and corresponding object files
TEST_SRC = tests/test0_sanity.cpp tests/test2_two_thread.cpp
TEST_OBJ = $(TEST_SRC:.cpp=.o)

# Define the library and object files
LIB_SRC = Thread.cpp uthreads.cpp
LIB_OBJ = $(LIB_SRC:.cpp=.o)

# Create the build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the library
$(LIBRARY): $(BUILD_DIR) $(LIB_OBJ)
	$(AR) $(ARFLAGS) $(LIB_DIR)/$(LIBRARY) $(LIB_OBJ)

# Compile the test source files
build/%.o: tests/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build the test executables
tests/test0_sanity: build/test0_sanity.o $(LIBRARY)
	$(CXX) $(CXXFLAGS) $< -L$(BUILD_DIR) -luthreads -o $@

tests/test2_two_thread: build/test2_two_thread.o $(LIBRARY)
	$(CXX) $(CXXFLAGS) $< -L$(BUILD_DIR) -luthreads -o $@

# Link all tests together
run_tests: tests/test0_sanity tests/test2_two_thread
	./tests/test0_sanity
	./tests/test2_two_thread

# Clean up build artifacts
clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/$(LIBRARY) tests/test0_sanity tests/test2_two_thread

.PHONY: all clean run_tests
