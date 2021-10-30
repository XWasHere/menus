// this file might give you cancer

#ifndef MENUS_SCRIPT_OPS_H
#define MENUS_SCRIPT_OPS_H

#define OPCODE_VAR            0x01
#define OPCODE_SET            0x02
#define OPCODE_INITIALIZE     0x03
#define OPCODE_END_INITIALIZE 0x04
#define OPCODE_INTERRUPT      0x05
#define OPCODE_RETURN         0x06
#define OPCODE_CALL           0x07
#define OPCODE_END_CALL_ARGS  0x08
#define OPCODE_CALL_ARG       0x09
#define OPCODE_FUNCTION       0x0A
#define OPCODE_END_FUNCTION   0x0B

#define WR_OP_VAR(b, p, l, name, type) \
{ \
    int t = 1 + strlen(name) + 1 + strlen(type) + 1; \
    l += t;\
    b = realloc(b, l); \
    b[p] = OPCODE_VAR; \
    strcpy(b + p + 1, name); \
    strcpy(b + p + 1 + strlen(name) + 1, type); \
    p += t; \
}

// please dont use this, use the per type ones instead because value isnt actually a string im just lazy
#define WR_OP_SET(b, p, l, name, value, size) \
{ \
    int t = 1 + strlen(name) + 1 + size; \
    l += t; \
    b = realloc(b, l); \
    b[p] = OPCODE_SET; \
    strcpy(b + p + 1, name); \
    memcpy(b + p + 1 + strlen(name) + 1, value, size); \
    p += t; \
}

#define WR_OP_SET_STRING(b, p, l, name, value) \
{ \
    char* v = malloc(3 + strlen(value) + 1); \
    v[0] = 0; \
    v[1] = 1; \
    memcpy(v + 2, value, strlen(value) + 1); \
    WR_OP_SET(b, p, l, name, v, 2 + strlen(value) + 1); \
    free(v); \
}

#define WR_OP_SET_INT32(b, p, l, name, value) \
{ \
    char* v = malloc(6); \
    memset(v, 0, 6); \
    v[0] = 0; \
    v[1] = 2; \
    v[2] = 0x000000FF & value >> 0; \
    v[3] = 0x0000FF00 & value >> 8; \
    v[4] = 0x00FF0000 & value >> 16; \
    v[5] = 0xFF000000 & value >> 24; \
    WR_OP_SET(b, p, l, name, v, 6); \
    free(v); \
}

#define WR_OP_INITIALIZE(b, p, l, name) \
{ \
    int t = 1 + strlen(name) + 1; \
    l += t; \
    b = realloc(b, l); \
    b[p] = OPCODE_INITIALIZE; \
    memcpy(b + p + 1, name, strlen(name) + 1); \
    p += t; \
}

#define WR_OP_END_INITIALIZE(b, p, l) \
{ \
    int t = 1; \
    l += t; \
    b[p] = OPCODE_END_INITIALIZE; \
    p += t; \
    b = realloc(b, l); \
}

#define WR_OP_INTERRUPT(b, p, l) \
{ \
    int t = 1; \
    l += t; \
    b = realloc(b, l); \
    b[p] = OPCODE_INTERRUPT; \
    p += t; \
}

#define WR_OP_RETURN(b, p, l) \
{ \
    int t = 1; \
    l += t; \
    b = realloc(b, l); \
    b[p] = OPCODE_RETURN; \
    p += t; \
}

#define WR_OP_CALL(b, p, l, name) \
{ \
    int t = 1 + strlen(name) + 1; \
    l += t; \
    b = realloc(b, l); \
    b[p] = OPCODE_CALL; \
    strcpy(b + p + 1, name); \
    p += t; \
}

#define WR_OP_END_CALL_ARGS(b, p, l) \
{ \
    int t = 1; \
    l += t; \
    b = realloc(b, l); \
    b[p] = OPCODE_END_CALL_ARGS; \
    p += t; \
}

#define WR_OP_CALL_ARG(b, p, l, value) \
{ \
    int t = 1 + strlen(value); \
    l += t; \
    b = realloc(b, l); \
    b[p] = OPCODE_CALL_ARG; \
    strcpy(b + p + 1, value); \
    p += t; \
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
    b  = realloc(b, l); \
    b[p] = OPCODE_FUNCTION; \
    strcpy(b + p + 1, name); \
    strcpy(b + p + 1 + strlen(name) + 1, type); \
    p += t; \
}

#define WR_OP_END_FUNCTION(b, p, l) \
{ \
    int t = 1; \
    l += t; \
    b  = realloc(b, l); \ 
    b[p] = OPCODE_END_FUNCTION; \
    p += t; \
}

#endif