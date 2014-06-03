# Top level Makefile,the project actually use cmake.
CMAKE = cmake
BUILD_DIR = build
DEBUG_VAR = -DCMAKE_BUILD_TYPE=Debug
SOURCE_DIR = $(CURDIR)

all: build

libuv:
	wget https://github.com/joyent/libuv/archive/v0.11.11.tar.gz
	tar xzvf v0.11.11.tar.gz
	cd libuv-0.11.11; \
	sh autogen.sh; \
	./configure --prefix=/usr; \
	make; \
	sudo make install
	rm v0.11.11.tar.gz
	rm -rf libuv-0.11.11

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

.PHONY: all libuv prepare build clean
