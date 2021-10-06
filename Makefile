CC        := clang
CC_ARGS   := -g3 -Wall

OBJECTS = \
	build/main.o \
	build/daemon.o \
	build/ipc.o \

.PHONY: build clean all

all: build

build/%.o: src/%.c
	$(CC) -c $< -o $@

build/menus: $(OBJECTS)
	$(CC) $(OBJECTS) -o build/menus $(CC_ARGS)

build: build/menus

clean:
	rm build/*

test:
	./menus_test build/menus
