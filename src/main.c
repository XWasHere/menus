#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <termios.h>

#define ALTBUF_ON()  printf("\e[?1049h")
#define ALTBUF_OFF() printf("\e[?1049l")

#define CURSOR_GOTO(row, col) printf("\e[%i;%iH", row, col)

#define CURSOR_SAVE()         printf("\e7")
#define CURSOR_LOAD()         printf("\e8")

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
    int   type;
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

int            id;
char           menusi[256];
char           menuso[256];

int main(int argc, char** argv) {
    id = getppid();
    sprintf(menusi, "/tmp/.menus.%i.i", id);
    sprintf(menuso, "/tmp/.menus.%i.o", id);
    if (strcmp(argv[1], "init") == 0) {
        if (fork() == 0) {
            menu_t *root = malloc(sizeof(menu_t));
            int choices[32];
            int choices_len = 0;

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
                    int cr = 0;
                    int cc = 0;
                    int outfd = open(menuso, O_WRONLY);
                    int selected = 0;
                    struct pollfd pfds[1];

                    pfds[0].fd     = infd;
                    pfds[0].events = POLLIN;

                    // save state
                    CURSOR_SAVE();
                    ALTBUF_ON();
                    
                    goto render;
                    
                    while (1) {
                        CURSOR_GOTO(0, 0);
                        printf("%i", poll(pfds, 1, -1));
                        
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
                                    if (c == 'B') {
                                        if (selected == choices_len-1) selected = 0;
                                        else selected++;
                                    } else if (c == 'A') {
                                        if (selected == 0) selected = choices_len-1;
                                        else selected--;
                                    }
                                }
                            } else if (c == 13) {
                                char tmp = '\x08';

                                write(outfd, &tmp, 1);
                                write(outfd, &root->items[choices[selected]]->name_len, 4);
                                write(outfd, root->items[choices[selected]]->name, root->items[choices[selected]]->name_len);

                                close(outfd);
                                close(infd);

                                idk:
                                infd = open(menusi, O_RDONLY);
                                pfds[0].fd = infd;

                                poll(pfds, 1, -1);
                                read(infd, &tmp, 1);

                                printf("%i\n", tmp);

                                if (tmp == 1) {
                                    ALTBUF_OFF();
                                    CURSOR_LOAD();
                                    close(outfd);
                                    exit(0);
                                } else if (tmp == 2) {
                                    outfd = open(menuso, O_WRONLY);
                                } else if (tmp == 9) { // get the pressed button
                                    outfd = open(menuso, O_WRONLY);
		                        write(outfd, &root->items[choices[selected]]->name_len, 4);
                                    write(outfd, root->items[choices[selected]]->name, root->items[choices[selected]]->name_len);
                                    
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
                                CURSOR_GOTO(cr, 0);
                                printf("%c %s %c\n",
                                    (choices[selected] == i) ? '>' : ' ', 
                                    root->items[i]->button->text,
                                    (choices[selected] == i) ? '<' : ' ');
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
                        
                        choices[choices_len] = root->itemc;
                        choices_len++;

                        button_t   *btn  = malloc(sizeof(button_t));
                        menuitem_t *item = malloc(sizeof(menuitem_t));
                        
                        item->type               = 0;
                        item->button             = btn;
                        item->name               = name;
                        item->name_len           = name_len;
                        btn->text                = "";
                        btn->text_len            = 1;
                        root->items              = realloc(root->items, sizeof(menuitem_t*) * root->itemc);
                        root->items[root->itemc] = item;
                        root->itemc++;
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
                                    root->items[i]->button->text     = target;
                                    root->items[i]->button->text_len = targetlen;
                                }
                            }
                        }
                    }
                } else {}
                free(msg);
                close(infd);
            }
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
            else		        exit(1);
        } else {
            perror("invalid test");
            exit(-1);
        }
    }
}
