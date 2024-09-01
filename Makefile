# Define compiler and flags
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I. -Itests
AR = ar
ARFLAGS = rcs
BUILD_DIR = build
LIB_DIR = $(BUILD_DIR)
LIBRARY = libuthreads.a

# Define source files
LIB_SRC = Thread.cpp uthreads.cpp
LIB_OBJ = $(LIB_SRC:.cpp=.o)

TEST_SRC = tests/test0_sanity.cpp tests/test2_two_thread.cpp tests/uthreads_tests.cpp
TEST_OBJ = $(TEST_SRC:.cpp=$(BUILD_DIR)/%.o)

# Create the build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the library
$(LIB_DIR)/$(LIBRARY): $(BUILD_DIR) $(LIB_OBJ)
	$(AR) $(ARFLAGS) $@ $(LIB_OBJ)

# Compile library source files
$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile test source files
$(BUILD_DIR)/%.o: tests/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build test executables
tests/test0_sanity: $(BUILD_DIR)/tests/test0_sanity.o $(LIB_DIR)/$(LIBRARY)
	$(CXX) $(CXXFLAGS) $< -L$(BUILD_DIR) -luthreads -o $@

tests/test2_two_thread: $(BUILD_DIR)/tests/test2_two_thread.o $(LIB_DIR)/$(LIBRARY)
	$(CXX) $(CXXFLAGS) $< -L$(BUILD_DIR) -luthreads -o $@

tests/uthreads_tests: $(BUILD_DIR)/tests/uthreads_tests.o $(LIB_DIR)/$(LIBRARY)
	$(CXX) $(CXXFLAGS) $< -L$(BUILD_DIR) -luthreads -o $@

# Link all tests together
run_tests: tests/test0_sanity tests/test2_two_thread tests/uthreads_tests
	./tests/test0_sanity
	./tests/test2_two_thread
	./tests/uthreads_tests

# Clean up build artifacts
clean:
	rm -f $(BUILD_DIR)/*.o $(LIB_DIR)/$(LIBRARY) tests/test0_sanity tests/test2_two_thread tests/uthreads_tests

.PHONY: all clean run_tests
