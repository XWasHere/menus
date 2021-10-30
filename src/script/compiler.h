#ifndef MENUS_SCRIPT_PARSER_H
#define MENUS_SCRIPT_PARSER_H

#include <stdint.h>

struct script_header {
    uint32_t version;
    uint32_t entry;
};
typedef struct script_header script_header_t;

char* compile(char* src);

#endif