 # Final binary
BIN = main

# Put all auto generated stuff to this build dir.
BUILD_DIR = ./build

 subsystem:
	cd cpp_driver && $(MAKE)

.PHONY : clean
clean :
    # This should remove all generated files.
	-rm -r $(BUILD_DIR)