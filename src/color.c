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

#include "color.h"
#include "ansi.h"

#include <stdio.h>
#include <stdlib.h>

color_t *read_hex_color(char *stuff) {
    color_t* c = malloc(sizeof(color_t));

    sscanf(stuff, "%2x%2x%2x", &c->r, &c->g, &c->b);

    return c;
}

void apply_fg(color_t *c) {
    TEXT_RGB_FG(c->r, c->g, c->b);
}

void apply_bg(color_t *c) {
    TEXT_RGB_BG(c->r, c->g, c->b);
}