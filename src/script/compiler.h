#ifndef MENUS_SCRIPT_PARSER_H
#define MENUS_SCRIPT_PARSER_H

#include <stdint.h>

struct script_header {
    uint32_t version;
};
typedef struct script_header script_header_t;

struct module {
    uint32_t length;
    char* code;
};
typedef struct module module_t;

struct module* menus_vm_compile(char* src);

#endif