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

int            id;
char           menusi[256];
char           menuso[256];

int main(int argc, char** argv) {
	if (argc == 1) goto help;
    id = getppid();
    sprintf(menusi, "/tmp/.menus.%i.i", id);
    sprintf(menuso, "/tmp/.menus.%i.o", id);
    if (strcmp(argv[1], "init") == 0) {
        if (fork() == 0) {
            daemon_main(menusi, menuso);
        } else {
            exit(0);
        }
    } else if (strcmp(argv[1], "exit") == 0) {
        int io = open(menusi, O_WRONLY);
        write(io,"\x01",1);
        close(io);
        exit(0);
    } else if (strcmp(argv[1], "display") == 0) {
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
                    char o = '\x07';
                    write(io, &o ,1);
                    break;
                } else {
                    char o = '\x06';
                    write(io, &o, 1);
                    write(io, &c, 1);
                }
            }

            if (pfds[1].revents & POLLIN) {
                char c;
                read(i, &c, 1);
                if (c == '\x08') { // button pressed
                    int namelen;
                    read(i, &namelen, 4);
                    char *name;
                    name = malloc(namelen+1);
                    memset(name, 0, namelen+1);
                    read(i, name, namelen);

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
    } else if (strcmp(argv[1], "button") == 0) {
        int io = open(menusi, O_WRONLY);
        write(io,"\x03\x01", 2);
        int len = strlen(argv[2]);
        write(io, &len, 4);
        write(io, argv[2], len);
        close(io);
        exit(0);
    } else if (strcmp(argv[1], "config") == 0) {
        if (strcmp(argv[3], "button.text") == 0) {
            int io  = open(menusi, O_WRONLY);
            write(io, "\x04\x01\x01", 3);
            int len = strlen(argv[2]);
            write(io, &len, 4);
            write(io, argv[2], len);
            int tlen= strlen(argv[4]);
            write(io, &tlen, 4);
            write(io, argv[4], tlen);
        } else if (strcmp(argv[3], "line") == 0) {
            int io = open(menusi, O_WRONLY);
            write(io, "\x04\x02\x01", 3);
            int l;
            sscanf(argv[4], "%i", &l);
            int tl = strlen(argv[2]);
            write(io, &tl,4);
            write(io, argv[2], tl);
            write(io, &l, 4);
        } else if (strcmp(argv[3], "col") == 0) {
            int io = open(menusi, O_WRONLY);
            write(io, "\x04\x02\x02", 3);
            int l;
            sscanf(argv[4], "%i", &l);
            int tl = strlen(argv[2]);
            write(io, &tl,4);
            write(io, argv[2], tl);
            write(io, &l, 4);
        } else if (strcmp(argv[3], "focus_up") == 0) {
            int io  = open(menusi, O_WRONLY);
            write(io, "\x04\x02\x03", 3);
            int len = strlen(argv[2]);
            write(io, &len, 4);
            write(io, argv[2], len);
            int tlen= strlen(argv[4]);
            write(io, &tlen, 4);
            write(io, argv[4], tlen);
        } else if (strcmp(argv[3], "focus_down") == 0) {
            int io  = open(menusi, O_WRONLY);
            write(io, "\x04\x02\x04", 3);
            int len = strlen(argv[2]);
            write(io, &len, 4);
            write(io, argv[2], len);
            int tlen= strlen(argv[4]);
            write(io, &tlen, 4);
            write(io, argv[4], tlen);
        } else if (strcmp(argv[3], "focus_right") == 0) {
            int io  = open(menusi, O_WRONLY);
            write(io, "\x04\x02\x05", 3);
            int len = strlen(argv[2]);
            write(io, &len, 4);
            write(io, argv[2], len);
            int tlen= strlen(argv[4]);
            write(io, &tlen, 4);
            write(io, argv[4], tlen);
        } else if (strcmp(argv[3], "focus_left") == 0) {
            int io  = open(menusi, O_WRONLY);
            write(io, "\x04\x02\x06", 3);
            int len = strlen(argv[2]);
            write(io, &len, 4);
            write(io, argv[2], len);
            int tlen= strlen(argv[4]);
            write(io, &tlen, 4);
            write(io, argv[4], tlen);
        } else if (strcmp(argv[3], "style.selected") == 0) {
            if (strcmp(argv[4], "arrows") == 0) {
                int io = open(menusi, O_WRONLY);
                write(io, "\x04\x02\x07\x01", 4);
                int len = strlen(argv[2]);
                write(io, &len, 4);
                write(io, argv[2], len);
            } else if (strcmp(argv[4], "highlight") == 0) {
                int io = open(menusi, O_WRONLY);
                write(io, "\x04\x02\x07\x02", 4);
                int len = strlen(argv[2]);
                write(io, &len, 4);
                write(io, argv[2], len);
            }
        }
    } else if (strcmp(argv[1], "test") == 0) {
        if (strcmp(argv[3], "button.waspressed") == 0) {
            int io  = open(menusi, O_WRONLY);
            int len;
            char *name;

            write(io, "\x09", 1);
            close(io);

            io = open(menuso, O_RDONLY);
            read(io, &len, 4);

            name = malloc(len+1);
            memset(name, 0, len+1);
            read(io, name, len+1);
            close(io);
        	if (strcmp(name, argv[2]) == 0) exit(0);
            else	              	        exit(1);
        } else {
            perror("invalid test");
            exit(-1);
        }
    } else {
		help:
		printf("%s [\"init\"|\"display\"|\"button\" name|\"config\" name property value|\"test\" name property]\n", argv[0]);
	}
}
