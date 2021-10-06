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
#include "ipc.h"

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

void write_int(int fd, int data) {
    write(fd, &data, 4);
}

int read_int(int fd) {
    int n;
    read(fd, &n, 4);
    return n;
}

void write_string(int fd, char *data) {
    int len = strlen(data);
    write(fd, &len, 4);
    write(fd, data, len);
}

char* read_string(int fd) {
    int len = read_int(fd);
    char* data = malloc(len+1);
    memset(data, 0, len+1);
    read(fd, data, len);
    return data;
}