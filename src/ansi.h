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
#ifndef MENUS_ANSI_H
#define MENUS_ANSI_H

#include <stdio.h>

// enable the alternate screen buffer
#define ALTBUF_ON() printf("\e[?1049h")

// disable the alternate screen buffer
#define ALTBUF_OFF() printf("\e[?1049l")

// remove text styling
#define TEXT_RESET() printf("\e[0m")

// inverts text
#define TEXT_INVERT() printf("\e[7m")

// sets the color
#define TEXT_RGB_FG(r, g, b) printf("\e[38;2;%i;%i;%im", r, g, b)
#define TEXT_RGB_BG(r, g, b) printf("\e[48;2;%i;%i;%im", r, g, b)

// moves the cursor
#define CURSOR_GOTO(row, col) printf("\e[%i;%iH", row, col)

// saves the cursor position
#define CURSOR_SAVE() printf("\e7")

// restores the cursor position
#define CURSOR_LOAD() printf("\e8")

// hides the cursor
#define CURSOR_HIDE() printf("\e[?25l")

// shows the cursor
#define CURSOR_SHOW() printf("\e[?25h")

#endif