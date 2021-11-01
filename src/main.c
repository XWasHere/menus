/*
    menus - uis for shell scripts	
    Copyright (C) 2021 xwashere

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>
#ifdef __APPLE__
    // X - due to not having an apple device, its up to contributors to manage this. im sorry for letting you down :pensive:
    #include <sys/types.h>
    #include <sys/stat.h>
#endif

#include "daemon.h"
#include "ipc.h"
#include "color.h"
#include "args.h"
#include "script/compiler.h"
#include "script/interpreter.h"

int            id;
char           menusi[256];
char           menuso[256];

void menus_init(int argc, char** argv) {
    if (fork() == 0) {
        daemon_main(menusi, menuso);
    } else {
        exit(0);
    }
}

void menus_exit(int argc, char** argv) {
    int io = open(menusi, O_WRONLY);
    write(io,"\x01",1);
    close(io);
    exit(0);
}

void menus_display(int argc, char** argv) {
    int io = open(menusi, O_WRONLY);
    write(io,"\x02",1);

    int i  = open(menuso, O_RDONLY);

    struct termios backup;
    struct termios tios;
    
    tcgetattr(0, &backup);
    tcgetattr(0, &tios);
    cfmakeraw(&tios);
    tcsetattr(0, 0, &tios);

    struct pollfd pfds[2];
    pfds[0].fd = 0;
    pfds[0].events = POLLIN;
    pfds[1].fd = i;
    pfds[1].events = POLLIN;

    while (1) {
        poll(pfds, 2, -1);

        if (pfds[0].revents == POLLIN) {
            char c;
            read(0, &c, 1);

            if (c == 3) {
                write(io, "\x07" ,1);
                break;
            } else {
                write(io, "\x06", 1);
                write(io, &c, 1);
            }
        }

        if (pfds[1].revents & POLLIN) {
            char c;
            read(i, &c, 1);
            if (c == '\x08') { // button pressed
                char *name = read_string(i);

                tcsetattr(0, 0, &backup);

                close(io);
                close(i);
                exit(1);
            }
        } else if (pfds[1].revents & POLLHUP) {
            exit(0);
        }
    }

    tcsetattr(0, 0, &backup);

    close(io);
    exit(0);
}

void menus_button(int argc, char **argv) {
    int io = open(menusi, O_WRONLY);
    write(io,"\x03\x01", 2);
    write_string(io, argv[2]);
    close(io);
    exit(0);
}

void menus_config(int argc, char** argv) {
    int io = open(menusi, O_WRONLY);
    if (strcmp(argv[3], "button.text") == 0) {
        write(io, "\x04\x01\x01", 3);
        write_string(io, argv[2]);
        write_string(io, argv[4]);
    } else if (strcmp(argv[3], "line") == 0) {
        write(io, "\x04\x02\x01", 3);
        int l;
        sscanf(argv[4], "%i", &l);
        write_string(io, argv[2]);
        write(io, &l, 4);
    } else if (strcmp(argv[3], "col") == 0) {
        write(io, "\x04\x02\x02", 3);
        int l;
        sscanf(argv[4], "%i", &l);
        write_string(io, argv[2]);
        write(io, &l, 4);
    } else if (strcmp(argv[3], "focus_up") == 0) {
        write(io, "\x04\x02\x03", 3);
        write_string(io, argv[2]);
        write_string(io, argv[4]);
    } else if (strcmp(argv[3], "focus_down") == 0) {
        write(io, "\x04\x02\x04", 3);
        write_string(io, argv[2]);
        write_string(io, argv[4]);
    } else if (strcmp(argv[3], "focus_right") == 0) {
        write(io, "\x04\x02\x05", 3);
        write_string(io, argv[2]);
        write_string(io, argv[4]);
    } else if (strcmp(argv[3], "focus_left") == 0) {
        write(io, "\x04\x02\x06", 3);
        write_string(io, argv[2]);
        write_string(io, argv[4]);
    } else if (strcmp(argv[3], "style.selected") == 0) {
        if (strcmp(argv[4], "arrows") == 0) {
            write(io, "\x04\x02\x07\x01", 4);
            write_string(io, argv[2]);
        } else if (strcmp(argv[4], "highlight") == 0) {
            write(io, "\x04\x02\x07\x02", 4);
            write_string(io, argv[2]);
        }
    } else if (strcmp(argv[3], "style.color.fg") == 0) {
        color_t* c = read_hex_color(argv[4]);
        write(io, "\x04\x02\x08", 3);
        write_string(io, argv[2]);
        write_color(io, c);
    } else if (strcmp(argv[3], "style.color.bg") == 0) {
        color_t* c = read_hex_color(argv[4]);
        write(io, "\x04\x02\x09", 3);
        write_string(io, argv[2]);
        write_color(io, c);
    } else if (strcmp(argv[3], "style.color.fg.selected") == 0) {
        color_t* c = read_hex_color(argv[4]);
        write(io, "\x04\x02\x0a", 3); // we hit 10 opcodes for this property group wow
        write_string(io, argv[2]);
        write_color(io, c);
    } else if (strcmp(argv[3], "style.color.bg.selected") == 0) {
        color_t* c = read_hex_color(argv[4]);
        write(io, "\x04\x02\x0b", 3);
        write_string(io, argv[2]);
        write_color(io, c);
    }
    close(menusi);
}

void menus_test(int argc, char** argv) {
    if (strcmp(argv[3], "pressed") == 0) {
        int io  = open(menusi, O_WRONLY);
        char *name;

        write(io, "\x09", 1);
        close(io);

        io = open(menuso, O_RDONLY);
        
        name = read_string(io);
        printf("%s", name);
    } else if (strcmp(argv[3], "button.waspressed") == 0) {
        int io  = open(menusi, O_WRONLY);
        char *name;

        write(io, "\x09", 1);
        close(io);

        io = open(menuso, O_RDONLY);
        
        name = read_string(io);
        
        close(io);
        if (strcmp(name, argv[2]) == 0) exit(0);
        else	              	        exit(1);
    } else {
        perror("invalid test");
        exit(-1);
    }
}

void menus_help(int argc, char** argv) {
    printf("%s [\"init\"|\"display\"|\"button\" name|\"config\" name property value|\"test\" name property]\n", argv[0]);
    exit(0);
}

int main(int argc, char** argv) {
    parse_args(argc, argv);
    if (arg_has_src) {
        module_t* m = compile(arg_src);
        load(m);
    }
    return 0;

	if (argc == 1) menus_help(argc, argv);
    id = getppid();
    sprintf(menusi, "/tmp/.menus.%i.i", id);
    sprintf(menuso, "/tmp/.menus.%i.o", id);
    if (strcmp(argv[1], "init") == 0)         menus_init(argc, argv);
    else if (strcmp(argv[1], "exit") == 0)    menus_exit(argc, argv);
    else if (strcmp(argv[1], "display") == 0) menus_display(argc, argv); 
    else if (strcmp(argv[1], "button") == 0)  menus_button(argc, argv);
    else if (strcmp(argv[1], "config") == 0)  menus_config(argc, argv);
    else if (strcmp(argv[1], "test") == 0)    menus_test(argc, argv);
    else                                      menus_help(argc, argv);
}