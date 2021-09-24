BUILD_DIR := ./build/
GCC       := gcc

.PHONY: build buildfolder clean all

all: build

buildfolder:
  mkdir --parent $(BUILD_DIR)

build: src/main.c buildfolder
  gcc -c src/main.c -o $(BUILD_DIR)/main.o
  gcc $(BUILD_DIR)/main.o -o $(BUILD_DIR)/menus
  
clean:
  rm build/*

test:
  ./menus_test $(BUILD_DIR)/menus
