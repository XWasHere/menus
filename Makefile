BUILD_DIR := ./build
CC        := clang
CC_ARGS   := -g3 -Wall

.PHONY: build buildfolder clean all

all: build

buildfolder:
	mkdir -p $(BUILD_DIR)

build: src/main.c src/daemon.c buildfolder
	$(CC) -c src/main.c -o $(BUILD_DIR)/main.o $(CC_ARGS)
	$(CC) -c src/daemon.c -o $(BUILD_DIR)/daemon.o $(CC_ARGS)
	$(CC) -c src/ipc.c -o $(BUILD_DIR)/ipc.o $(CC_ARGS)
	$(CC) $(BUILD_DIR)/main.o $(BUILD_DIR)/daemon.o $(BUILD_DIR)/ipc.o -o $(BUILD_DIR)/menus $(CC_ARGS)

clean:
	rm build/*

test:
	./menus_test $(BUILD_DIR)/menus
