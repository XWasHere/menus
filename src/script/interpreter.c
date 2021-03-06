#include <stdint.h>
#include <stdio.h>

#include "ops.h"
#include "interpreter.h"
#include "compiler.h"

struct function {
    char*    name;
    int      native;
    uint32_t addr;
    void*    native_addr; 
};
typedef struct function function_t;

struct callarg {
    int type;
    union {
        char* refv;
        char* stringv;
        int   intv;
    };
};
typedef struct callarg callarg_t;

function_t* modfuncs;
int         modfcount;
uint32_t    modip;
char*       modcode;
int         modlen;
int         modnativedist;
char*       modcalling;
int*        modstack;
int         modcallargc;
callarg_t*  modcallargv;

void menus_vm_native_display() {
    printf("this would display crap\n");
}

void menus_vm_load(module_t* module) {
    modcode = module->code;
    modlen  = module->length;

    modip     = 4;
    modfcount = 0;
    modfuncs  = malloc(sizeof(function_t) * 1);
    modstack  = malloc(256);
    modnativedist = 0;

    modfuncs[0].native_addr = &menus_vm_native_display;
    modfuncs[0].native = 1;
    modfuncs[0].name = "display";

    modfcount = 1;

    while (modip < modlen) {
        menus_vm_exec_instr();
    }

    menus_vm_call("main");
}

int menus_vm_exec_instr() {
    if (modip > modlen) return 0;
    if (modcode[modip] == OPCODE_VAR) {
        char* name = modcode + modip + 1;
        char* type = modcode + modip + 1 + strlen(name) + 1;
        printf("DECLARE VAR %s AS %s\n", name, type);
        modip += strlen(name) + 1 + strlen(type) + 1;
    } else if (modcode[modip] == OPCODE_SET) {
        char* name = modcode + modip + 1;
        modip += strlen(name) + 1;
        if (modcode[modip+1] == 0) {
            if (modcode[modip+2] == 1) {
                char* value = modcode + modip + 2;
                printf("SET STRING %s TO \"%s\"\n", name, value);
                modip += 2 + strlen(value);
            } else if (modcode[modip+2] == 2) {
                int value = modcode[modip+3] | modcode[modip+4] >> 8 | modcode[modip+5] >> 16 | modcode[modip+6] >> 24;
                printf("SET INT %s TO %i\n", name, value);
                modip += 6;
            } else if (modcode[modip+2] == 3) {
                char* value = modcode + modip + 2;
                printf("SET REF %s TO %s\n", name, value);
                modip += 2 + strlen(value);
            }
        }
    } else if (modcode[modip] == OPCODE_INITIALIZE) {
        char* name = modcode + modip + 1;
        printf("INITIALIZE %s\n", name);
        modip += strlen(name);
    } else if (modcode[modip] == OPCODE_END_INITIALIZE) {
        printf("END_INITIALIZE\n");
    } else if (modcode[modip] == OPCODE_INTERRUPT) {
        printf("INTERRUPT\n");
        modip++;
        return 0;
    } else if (modcode[modip] == OPCODE_RETURN) {
        printf("RETURN\n");
        if (modnativedist == 0) {
            modip++;
            return 0;
        }
    } else if (modcode[modip] == OPCODE_CALL) {
        char* name = modcode + modip + 1;
        modcalling = name;
        printf("CALL %s\n", name);
    } else if (modcode[modip] == OPCODE_END_CALL_ARGS) {
        modcallargv = realloc(modcallargv, sizeof(callarg_t) * (modcallargc + 1));
        for (int i = 0; i < modfcount; i++) {
            if (strcmp(modfuncs[i].name, modcalling) == 0) {
                if (modfuncs[i].native == 0) {
                    menus_vm_call(modcalling);
                } else {
                    ((void(*)())modfuncs[i].native_addr)();
                }
            }
        }
        printf("%s\n", modcalling);
        modcallargc = 0;
        free(modcallargv);
        modcallargv = malloc(1);
        printf("END_ARGS\n");
    } else if (modcode[modip] == OPCODE_CALL_ARG) {
        modcallargv = realloc(modcallargv, sizeof(callarg_t) * (modcallargc + 1));
        if (modcode[modip+1] == 0) {
            if (modcode[modip+2] == 1) {
                char* value = modcode + modip + 2;
                printf("ARG STRING \"%s\"\n", value);
                modip += 2 + strlen(value);
            } else if (modcode[modip+2] == 2) {
                int value = modcode[modip+3] | modcode[modip+4] >> 8 | modcode[modip+5] >> 16 | modcode[modip+6] >> 24;
                printf("ARG INT %i\n", value);
                modip += 6;
            } else if (modcode[modip+2] == 3) {
                char* value = modcode + modip + 2;
                printf("ARG REF %s\n", value);
                modcallargv[modcallargc].type = 3;
                modcallargv[modcallargc].refv = value;
                modcallargc++;
                modip += 2 + strlen(value);
            }
        }
    } else if (modcode[modip] == OPCODE_FUNCTION) {
        char* name = modcode + modip + 1;
        printf("FUNCTION %s\n", name);
        modfuncs = realloc(modfuncs, sizeof(function_t) * (modfcount + 1));
        modfuncs[modfcount].name = name;
        modfuncs[modfcount].native = 0;
        modfuncs[modfcount].addr = modip + strlen(name) + 2;
        modfcount++;
        modip += 2 + strlen(name);
    } else if (modcode[modip] == OPCODE_END_FUNCTION) {
        printf("END_FUNCTION\n");
        if (modnativedist == 0) {
            modip++;
            return 0;
        } 
    }
    modip++;
    return 1;
}


void menus_vm_call(char* name) {
    for (int i = 0; i < modfcount; i++) {
        if (strcmp(modfuncs[i].name, name) == 0) {
            modip = modfuncs[i].addr;
            while (menus_vm_exec_instr()) {}
            break;
        }
    }
}