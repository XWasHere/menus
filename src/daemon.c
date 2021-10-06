#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>

#include "daemon.h"
#include "ansi.h"

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
    struct color    *bg;

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

struct color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t alt; // for terminals that dont support RGB colors :>
};
typedef struct color color_t;

int daemon_main(char *menusi, char *menuso) {
    menu_t *root = malloc(sizeof(menu_t));
    menuitem_t *selected;
    menuitem_t *defaultc;

    char default_selected_style = 1;

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
                        write(outfd, &selected->name_len, 4);
                        write(outfd, selected->name, selected->name_len);

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
                            write(outfd, &selected->name_len, 4);
                            write(outfd, selected->name, selected->name_len);
                            
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
                        CURSOR_GOTO(root->items[i]->line, root->items[i]->col);
                        char style;
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
                            TEXT_RESET();
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
                int name_len;
                read(infd, &name_len, 4);
                char *name = malloc(name_len+1);
                memset(name, 0, name_len+1);
                read(infd, name, name_len);
                
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
                root->items              = realloc(root->items, sizeof(menuitem_t*) * root->itemc);
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
                    int targetlen;
                    int valuelen;
                    char *target;
                    char *value;

                    read(infd, &targetlen, 4);
                    target = malloc(targetlen + 1);
                    memset(target, 0, targetlen + 1);
                    read(infd, target, targetlen);
                    read(infd, &valuelen, 4);
                    value = malloc(valuelen + 1);
                    memset(value, 0, valuelen + 1);
                    read(infd, value, valuelen);

                    for (int i = 0; i < root->itemc; i++) {
                        if (strcmp(root->items[i]->name, target)==0) {
                            root->items[i]->button->text     = value;
                            root->items[i]->button->text_len = valuelen;
                        }
                    }
                }
            } else if (targettype == 2) {
                if (param == 1) {
                    int targetlen;
                    int value;
                    char *target;
                    
                    read(infd, &targetlen, 4);
                    target = malloc(targetlen + 1);
                    memset(target, 0, targetlen + 1);
                    read(infd, target, targetlen);
                    read(infd, &value, 4);

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
                    int targetlen;
                    int value;
                    char *target;
                    
                    read(infd, &targetlen, 4);
                    target = malloc(targetlen + 1);
                    memset(target, 0, targetlen + 1);
                    read(infd, target, targetlen);
                    read(infd, &value, 4);

                    menuitem_t *t;

                    for (int i = 0; i < root->itemc; i++) {
                        if (strcmp(root->items[i]->name, target)==0) {
                            t = root->items[i];
                            t->col=value;
                        }
                    }
                } else if (param == 3) { 
                    int targetlen;
                    int valuelen;
                    char *target;
                    char *value;

                    read(infd, &targetlen, 4);
                    target = malloc(targetlen + 1);
                    memset(target, 0, targetlen + 1);
                    read(infd, target, targetlen);
                    read(infd, &valuelen, 4);
                    value = malloc(valuelen + 1);
                    memset(value, 0, valuelen + 1);
                    read(infd, value, valuelen);

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
                    int targetlen;
                    int valuelen;
                    char *target;
                    char *value;

                    read(infd, &targetlen, 4);
                    target = malloc(targetlen + 1);
                    memset(target, 0, targetlen + 1);
                    read(infd, target, targetlen);
                    read(infd, &valuelen, 4);
                    value = malloc(valuelen + 1);
                    memset(value, 0, valuelen + 1);
                    read(infd, value, valuelen);

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
                    int targetlen;
                    int valuelen;
                    char *target;
                    char *value;

                    read(infd, &targetlen, 4);
                    target = malloc(targetlen + 1);
                    memset(target, 0, targetlen + 1);
                    read(infd, target, targetlen);
                    read(infd, &valuelen, 4);
                    value = malloc(valuelen + 1);
                    memset(value, 0, valuelen + 1);
                    read(infd, value, valuelen);

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
                    int targetlen;
                    int valuelen;
                    char *target;
                    char *value;

                    read(infd, &targetlen, 4);
                    target = malloc(targetlen + 1);
                    memset(target, 0, targetlen + 1);
                    read(infd, target, targetlen);
                    read(infd, &valuelen, 4);
                    value = malloc(valuelen + 1);
                    memset(value, 0, valuelen + 1);
                    read(infd, value, valuelen);

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
                    int targetlen;
                    char* target;
                    char style;

                    read(infd, &style, 1);
                    read(infd, &targetlen, 4);
                    target = malloc(targetlen + 1);
                    memset(target, 0, targetlen + 1);
                    read(infd, target, targetlen);

                    if (strcmp(target, "@default") == 0) {
                        default_selected_style = style;
                    } else {
                        for (int i = 0; i < root->itemc; i++) {
                            if (strcmp(root->items[i]->name, target) == 0) {
                                root->items[i]->selected_style = style;
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