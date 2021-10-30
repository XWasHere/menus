// this file might give you cancer

#ifndef MENUS_SCRIPT_OPS_H
#define MENUS_SCRIPT_OPS_H

#define WR_OP_VAR(b, p, l, name, type) \
{ \
    int t = 1 + strlen(name) + strlen(type); \
    l += t;\
    p += t; \
    b = realloc(b, l); \
}

#endif