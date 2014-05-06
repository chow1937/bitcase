# Top level Makefile,the project actually use cmake.
CMAKE = cmake
BUILD_DIR = build
DEBUG_VAR = -DCMAKE_BUILD_TYPE=Debug
SOURCE_DIR = $(CURDIR)

all: build

prepare:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR); $(CMAKE) $(CMAKE_VARS) $(CURDIR)

debug-prepare:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR); $(CMAKE) $(CMAKE_VARS) $(DEBUG_VAR) $(CURDIR)

build: prepare
	cd $(BUILD_DIR); $(MAKE)

debug: debug-prepare
	cd $(BUILD_DIR); $(MAKE)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all prepare build clean
