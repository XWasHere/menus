// this file might give you cancer

#ifndef MENUS_SCRIPT_OPS_H
#define MENUS_SCRIPT_OPS_H

#define WR_OP_VAR(b, p, l, name, type) \
{ \
    int t = 1 + strlen(name) + 1 + strlen(type) + 1; \
    l += t;\
    p += t; \
    b = realloc(b, l); \
}

// please dont use this, use the per type ones instead because value isnt actually a string im just lazy
#define WR_OP_SET(b, p, l, name, value) \
{ \
    int t = 1 + strlen(name) + 1 + strlen(value); \
    l += t; \
    p += t; \
    b = realloc(b, l); \
}

#define WR_OP_SET_STRING(b, p, l, name, value) \
{ \
    char* v = malloc(2 + strlen(value) + 1); \
    v[0] = 0; \
    v[1] = 1; \
    memcpy(v + 2, value, strlen(value) + 1); \
    WR_OP_SET(b, p, l, name, v); \
    free(v); \
}

#endif