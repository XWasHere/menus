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
#ifndef MENUS_COLOR_H
#define MENUS_COLOR_H

#include <stdint.h>

struct color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t alt; // for terminals that dont support RGB colors :>
};
typedef struct color color_t;

color_t *read_hex_color(char*);
void apply_fg(color_t* color);
void apply_bg(color_t* color);

#endif