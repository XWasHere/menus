CC        ?= clang
CC_ARGS   ?= -g3 -Wall
NASM      ?= nasm
NASM_ARGS ?= -f elf64

OBJECTS = \
	build/main.o \
	build/daemon.o \
	build/ipc.o \
	build/color.o \
	build/args.o \
	build/script/compiler.o \
	build/script/interpreter.o

.PHONY: build clean all

all: build

include $(OBJECTS:.o=.o.d)

build/%.o.d: src/%.c
	$(CC) $(CC_ARGS) -MM $< -MT "$(@:.o.d=.o)" -o $@

build/%.o.d: src/%.s
	
build/%.o: src/%.s
	$(NASM) $(NASM_ARGS) $< -o $@

build/%.o: src/%.c
	$(CC) $(CC_ARGS) -c $< -o $@

build/menus: $(OBJECTS)
	$(CC) $(OBJECTS) -o build/menus $(CC_ARGS)

build: build/menus

clean:
	rm -rf build/*
	mkdir build/script

test:
	./menus_test build/menus