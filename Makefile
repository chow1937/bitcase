# Top level Makefile,the project actually use cmake.
CMAKE = cmake
BUILD_DIR = build
SOURCE_DIR = $(CURDIR)

all: build

prepare:
	mkdir -p $(BULID_DIR)
	cd $(BUILD_DIR); $(CMAKE) $(CMAKE_VARS) $(CURDIR)

build: prepare
	cd $(BUILD_DIR); $(MAKE)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all prepare build clean
