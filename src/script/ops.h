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

#define WR_OP_SET_INT32(b, p, l, name, value) \
{ \
    char* v = malloc(6); \
    v[0] = 0; \
    v[1] = 2; \
    ((int*)v+2)[0] = value; \
    WR_OP_SET(b, p, l, name, v); \
    free(v); \
}

#define WR_OP_INITIALIZE(b, p, l, name) \
{ \
    int t = 1 + strlen(name) + 1; \
    l += t; \
    p += t; \
    b = realloc(b, l); \
}

#define WR_OP_END_INITIALIZE(b, p, l) \
{ \
    int t = 1; \
    l += t; \
    p += t; \
    b = realloc(b, l); \
}

#define WR_OP_INTERRUPT(b, p, l) \
{ \
    int t = 1; \
    l += t; \
    p += t; \
    b = realloc(b, l); \
}

#define WR_OP_RETURN(b, p, l) \
{ \
    int t = 1; \
    l += t; \
    p += t; \
    b = realloc(b, 1); \
}

#define WR_OP_CALL(b, p, l, name) \
{ \
    int t = 1 + strlen(name) + 1; \
    l += t; \
    p += t; \
    b = realloc(b, l); \
}

#define WR_OP_CALL_ARGS_END(b, p, l) \
{ \
    int t = 1; \
    l += t; \
    p += t; \
    b = realloc(b, l); \
}

#define WR_OP_CALL_ARG(b, p, l, value) \
{ \
    int t = 1 + strlen(value); \
    l += t; \
    p += t; \
    b = realloc(b, l); \
}

#define WR_OP_CALL_ARG_REF(b, p, l, name) \
{ \
    char* v = malloc(2 + strlen(name) + 1); \
    v[0] = 0; \
    v[1] = 3; \
    memcpy(v + 2, name, strlen(name) + 1); \
    WR_OP_CALL_ARG(b, p, l, v); \
    free(v); \
}

#define WR_OP_FUNCTION(b, p, l, name, type) \
{ \
    int t = 1 + strlen(name) + 1 + strlen(type) + 1; \
    l += t; \
    p += t; \
    b  = realloc(b, l); \
}

#define WR_OP_END_FUNCTION(b, p, l) \
{ \
    int t = 1; \
    l += t; \
    p += 2; \
    b  = realloc(b, 1); \ 
}

#endif