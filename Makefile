# Final binary
BIN = main

# Put all auto generated stuff to this build dir.
BUILD_DIR = ./build

# Compiler
CC=g++
CFLAGS=-I$(IDIR) 
POSTFLAGS=-lpthread

# List of all .c/.cpp source files.
SRC = xavna_controller.cpp $(wildcard libxavna/*.cpp)

# All .o files go to build dir.
OBJ = $(SRC:%.cpp=$(BUILD_DIR)/%.o)

# Gcc/Clang will create these .d files containing dependencies.
DEP = $(OBJ:%.o=%.d)

# Default target named after the binary.
$(BIN) : $(BUILD_DIR)/$(BIN)

# Actual target of the binary - depends on all .o files.
$(BUILD_DIR)/$(BIN) : $(OBJ)
    # Create build directories - same structure as sources.
	mkdir -p $(@D)
    # Just link all the object files.
	$(CC) $^ -o $@ $(POSTFLAGS) 
	
# Include all .d files
-include $(DEP)

# Build target for every single object file.
# The potential dependency on header files is covered
# by calling `-include $(DEP)`.
$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
    # The -MMD flags additionaly creates a .d file with
    # the same name as the .o file.
	$(CC) $(CFLAGS) -MMD -c $< -o $@ $(POSTFLAGS)

.PHONY : clean
clean :
    # This should remove all generated files.
	-rm $(BUILD_DIR)/$(BIN) $(OBJ) $(DEP)