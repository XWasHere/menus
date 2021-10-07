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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>

#include "daemon.h"
#include "ansi.h"
#include "ipc.h"
#include "color.h"

struct cursor_state {
    int line;
    int column;
};
typedef struct cursor_state cursor_state_t;

struct button {
    int   text_len;
    char *text;
};
typedef struct button button_t;

struct menuitem {
    int   name_len;
    char *name;
    int   line;
    int   col;
    char  selected_style;
    int   type;
    
    struct color    *fg;
    struct color    *fg_selected;
    struct color    *bg;
    struct color    *bg_selected;

    int              manual_focus;
    struct menuitem *cdown;
    struct menuitem *cup;
    struct menuitem *cright;
    struct menuitem *cleft;

    union {
        button_t *button;
    };
};
typedef struct menuitem menuitem_t;

struct menu {
    int          itemc;
    menuitem_t **items;
};
typedef struct menu menu_t;

int daemon_main(char *menusi, char *menuso) {
    menu_t *root = malloc(sizeof(menu_t));
    menuitem_t *selected;
    menuitem_t *defaultc;

    char default_selected_style = 1;
    
    color_t* default_fg = 0;
    color_t* default_fg_selected = 0;
    color_t* default_bg = 0;
    color_t* default_bg_selected = 0;
    
    int nextline = 1;

    mkfifo(menusi, 0777);
    mkfifo(menuso, 0777);
    while (1) {
        char *msg    = malloc(256);
        memset(msg, 0, 256);
        int   infd   = open(menusi, O_RDONLY);
        int   outfd  = 0;
        int   msglen = read(infd, msg, 1);
        if (msg[0] == 1) {
            exit(0);
        } else if (msg[0] == 2) {
            // set some crap
            selected = defaultc;
            int cr = 0;
            int cc = 0;
            int outfd = open(menuso, O_WRONLY);

            struct pollfd pfds[1];

            pfds[0].fd     = infd;
            pfds[0].events = POLLIN;

            // save state
            CURSOR_SAVE();
            ALTBUF_ON();
            
            goto render;
            
            while (1) {
                CURSOR_GOTO(0, 0);
                poll(pfds, 1, -1);
                
                // input handling

                char op;
                read(infd , &op, 1);

                if (op == '\x06') {
                    char c;
                    read(infd, &c, 1);
                    if (c == '\e') {
                        read(infd, &op, 1);
                        read(infd, &c,  1);
                        if (c == '[') {
                            read(infd, &op, 1);
                            read(infd, &c , 1);
                            if (c == 'A') {
                                selected = selected->cup;
                            } else if (c == 'B') {
                                selected = selected->cdown;
                            } else if (c == 'C') {
                                selected = selected->cright;
                            } else if (c == 'D') {
                                selected = selected->cleft;
                            }
                        }
                    } else if (c == 13) {
                        char tmp = '\x08';

                        write(outfd, &tmp, 1);
                        write_string(outfd, selected->name);

                        close(outfd);
                        close(infd);

                        idk:
                        infd = open(menusi, O_RDONLY);
                        pfds[0].fd = infd;

                        poll(pfds, 1, -1);
                        read(infd, &tmp, 1);

                        if (tmp == 1) {
                            ALTBUF_OFF();
                            CURSOR_LOAD();
                            close(outfd);
                            exit(0);
                        } else if (tmp == 2) {
                            outfd = open(menuso, O_WRONLY);
                        } else if (tmp == 9) { // get the pressed button
                            outfd = open(menuso, O_WRONLY);
                            write_string(outfd,selected->name);
                            
                            close(infd);
                            close(outfd);
                            
                            goto idk;
                        }
                    }
                } else if (op == '\x07') {
                    goto stop;
                }

                render:
                // render
                CURSOR_GOTO(1, 1);
                cr = 1;
                cc = 1;
                for (int i = 0; i < root->itemc; i++) {
                    if (root->items[i]->type == 0) {
                        TEXT_RESET();
                        CURSOR_GOTO(root->items[i]->line, root->items[i]->col);
                        char style;
                        if (root->items[i] == selected) {
                            if (root->items[i]->fg_selected != 0) {
                                apply_fg(root->items[i]->fg_selected);
                            } else if (default_fg_selected != 0) {
                                apply_fg(default_fg_selected);
                            }
                            if (root->items[i]->bg_selected != 0) {
                                apply_bg(root->items[i]->bg_selected);
                            } else if (default_bg_selected != 0) {
                                apply_bg(default_bg_selected);
                            }
                        } else {
                            if (root->items[i]->fg != 0) {
                                apply_fg(root->items[i]->fg);
                            } else if (default_fg != 0) {
                                apply_fg(default_fg);
                            }
                            if (root->items[i]->bg != 0) {
                                apply_bg(root->items[i]->bg);
                            } else if (default_bg != 0) {
                                apply_bg(default_bg);
                            }
                        }
                        if (root->items[i]->selected_style == 0) {
                            style = default_selected_style;
                        } else {
                            style = root->items[i]->selected_style;
                        }
                        if (style == 1) {
                            printf("%c %s %c\n",
                                (root->items[i] == selected) ? '>' : ' ', 
                                root->items[i]->button->text,
                                (root->items[i] == selected) ? '<' : ' ');
                        } else if (style == 2) {
                            if (root->items[i] == selected) {
                                TEXT_INVERT();
                            }
                            printf("%s\n", root->items[i]->button->text);
                            if (root->items[i] == selected) {
                                TEXT_INVERT();
                            }
                        }
                        cr++;
                    }
                }
            }

            stop:
            close(outfd);
            stop2:
            // load state
            ALTBUF_OFF();
            CURSOR_LOAD();
        } else if (msg[0] == 3) {
            char type;
            read(infd, &type, 1);
            if (type == 1) {
                char *name     = read_string(infd);
                int   name_len = strlen(name);
                
                button_t   *btn  = malloc(sizeof(button_t));
                menuitem_t *item = malloc(sizeof(menuitem_t));
                
                item->type               = 0;
                item->button             = btn;
                item->name               = name;
                item->name_len           = name_len;
                item->line               = nextline;
                if (root->itemc) {
                    root->items[root->itemc-1]->cdown = item;
                    root->items[0]->cup  = item;
                    item->cdown          = root->items[0];
                    item->cup            = root->items[root->itemc-1];
                } else {
                    defaultc             = item;
                    item->cdown          = item;
                    item->cup            = item;
                }
                item->cright             = item;
                item->cleft              = item;
                item->selected_style     = 0;
                item->manual_focus       = 0;
                btn->text                = "";
                btn->text_len            = 1;
                root->items              = realloc(root->items, sizeof(menuitem_t*) * (root->itemc+1));
                root->items[root->itemc] = item;
                root->itemc++;

                nextline++;
            }
        } else if (msg[0] == 4) {
            char targettype;
            char param;
            read(infd, &targettype, 1);
            read(infd, &param,      1);
            if (targettype == 1) {
                if (param == 1) {
                    char *target;
                    char *value;

                    target = read_string(infd);
                    value  = read_string(infd);

                    for (int i = 0; i < root->itemc; i++) {
                        if (strcmp(root->items[i]->name, target)==0) {
                            root->items[i]->button->text     = value;
                            root->items[i]->button->text_len = strlen(value);
                        }
                    }
                }
            } else if (targettype == 2) {
                if (param == 1) {
                    int value;
                    char *target;
                    
                    target = read_string(infd);
                    value = read_int(infd);

                    menuitem_t *t;

                    for (int i = 0; i < root->itemc; i++) {
                        if (strcmp(root->items[i]->name, target)==0) {
                            t = root->items[i];
                            t->line=value;
                        }
                    }

                    if (!t->manual_focus) {
                        menuitem_t *top   = 0;
                        menuitem_t *bottom= 0;
                        menuitem_t *above = 0;
                        menuitem_t *below = 0;

                        for (int i = 0; i < root->itemc; i++) {
                            if (root->items[i]->line < value) {
                                if (above == 0) {
                                    above = root->items[i];
                                } else if (root->items[i]->line > above->line) {
                                    above = root->items[i];
                                }
                            }
                            if (root->items[i]->line > value) {
                                if (below == 0) {
                                    below = root->items[i];
                                } else if (root->items[i]->line < below->line) {
                                    below = root->items[i];
                                }
                            }
                            if (top == 0) {
                                top = root->items[i];
                            } else if (root->items[i]->line < top->line) {
                                top = root->items[i];
                            }
                            if (bottom == 0) {
                                bottom = root->items[i];
                            } else if (root->items[i]->line > bottom->line) {
                                bottom = root->items[i];
                            }
                        }

                        if (above == 0) {
                            above = bottom;
                        }
                        if (below == 0) {
                            below = top;
                        }
                        
                        above->cdown = t;
                        t->cup       = above;
                        below->cup   = t;
                        t->cdown     = below;
                    }
                } else if (param == 2) {
                    int value;
                    char *target;
                    
                    target = read_string(infd);
                    value = read_int(infd);

                    menuitem_t *t;

                    for (int i = 0; i < root->itemc; i++) {
                        if (strcmp(root->items[i]->name, target)==0) {
                            t = root->items[i];
                            t->col=value;
                        }
                    }
                } else if (param == 3) {
                    char *target;
                    char *value;

                    target = read_string(infd);
                    value = read_string(infd);

                    for (int i = 0; i < root->itemc; i++) {
                        if (strcmp(root->items[i]->name, target)==0) {
                            for (int ii = 0; ii < root->itemc; ii++) {
                                if (strcmp(root->items[ii]->name, value)==0) {
                                    root->items[i]->cup = root->items[ii];
                                    root->items[i]->manual_focus = 1;
                                }
                            }
                        }
                    }
                } else if (param == 4) { 
                    char *target;
                    char *value;

                    target = read_string(infd);
                    value = read_string(infd);

                    for (int i = 0; i < root->itemc; i++) {
                        if (strcmp(root->items[i]->name, target)==0) {
                            for (int ii = 0; ii < root->itemc; ii++) {
                                if (strcmp(root->items[ii]->name, value)==0) {
                                    root->items[i]->cdown = root->items[ii];
                                    root->items[i]->manual_focus = 1;
                                }
                            }
                        }
                    }
                } else if (param == 5) {
                    char *target;
                    char *value;

                    target= read_string(infd);
                    value = read_string(infd);

                    for (int i = 0; i < root->itemc; i++) {
                        if (strcmp(root->items[i]->name, target)==0) {
                            for (int ii = 0; ii < root->itemc; ii++) {
                                if (strcmp(root->items[ii]->name, value)==0) {
                                    root->items[i]->cright = root->items[ii];
                                    root->items[i]->manual_focus = 1;
                                }
                            }
                        }
                    }
                } else if (param == 6) {
                    char *target;
                    char *value;

                    target = read_string(infd);
                    value = read_string(infd);

                    for (int i = 0; i < root->itemc; i++) {
                        if (strcmp(root->items[i]->name, target)==0) {
                            for (int ii = 0; ii < root->itemc; ii++) {
                                if (strcmp(root->items[ii]->name, value)==0) {
                                    root->items[i]->cleft = root->items[ii];
                                    root->items[i]->manual_focus = 1;
                                }
                            }
                        }
                    }
                } else if (param == 7) {
                    char* target;
                    char style;

                    read(infd, &style, 1);
                    target = read_string(infd);

                    if (strcmp(target, "@default") == 0) {
                        default_selected_style = style;
                    } else {
                        for (int i = 0; i < root->itemc; i++) {
                            if (strcmp(root->items[i]->name, target) == 0) {
                                root->items[i]->selected_style = style;
                            }
                        }
                    }
                } else if (param == 8) {
                    char* target = read_string(infd);
                    color_t* c = read_color(infd);
                    if (strcmp(target, "@default") == 0) {
                        default_fg = c;
                    } else {
                        for (int i = 0; i < root->itemc; i++) {
                            if (strcmp(root->items[i]->name, target) == 0) {
                                root->items[i]->fg = c;
                            }
                        }
                    }
                } else if (param == 9) {
                    char* target = read_string(infd);
                    color_t* c = read_color(infd);
                    if (strcmp(target, "@default") == 0) {
                        default_bg = c;
                    } else {
                        for (int i = 0; i < root->itemc; i++) {
                            if (strcmp(root->items[i]->name, target) == 0) {
                                root->items[i]->bg = c;
                            }
                        }
                    }
                } else if (param == 10) {
                    char* target = read_string(infd);
                    color_t* c = read_color(infd);
                    if (strcmp(target, "@default") == 0) {
                        default_fg_selected = c;
                    } else {
                        for (int i = 0; i < root->itemc; i++) {
                            if (strcmp(root->items[i]->name, target) == 0) {
                                root->items[i]->fg_selected = c;
                            }
                        }
                    }
                } else if (param == 11) {
                    char* target = read_string(infd);
                    color_t* c = read_color(infd);
                    if (strcmp(target, "@default") == 0) {
                        default_bg_selected = c;
                    } else {
                        for (int i = 0; i < root->itemc; i++) {
                            if (strcmp(root->items[i]->name, target) == 0) {
                                root->items[i]->bg_selected = c;
                            }
                        }
                    }
                }
            }
        }
        free(msg);
        close(infd);
    }
}