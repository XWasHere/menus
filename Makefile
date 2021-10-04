BUILD_DIR := ./build/
GCC       := gcc
GCC_ARGS  := -g3

.PHONY: build buildfolder clean all

all: build

buildfolder:
	mkdir -p $(BUILD_DIR)

build: src/main.c buildfolder
	gcc -c src/main.c -o $(BUILD_DIR)/main.o  $(GCC_ARGS)
	gcc $(BUILD_DIR)/main.o -o $(BUILD_DIR)/menus $(GCC_ARGS)

clean:
	rm build/*

test:
	./menus_test $(BUILD_DIR)/menus
