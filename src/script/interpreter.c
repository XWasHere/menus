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

function_t* modfuncs;
int         modfcount;
uint32_t    modip;
char*       modcode;
int         modlen;

void load(module_t* module) {
    modcode = module->code;
    modlen  = module->length;

    modip     = 4;
    modfcount = 0;
    modfuncs  = malloc(1);
    
    while (modip < modlen) {
        execinstr();
    }
}

void execinstr() {
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
    } else if (modcode[modip] == OPCODE_RETURN) {
        printf("RETURN\n");
    } else if (modcode[modip] == OPCODE_CALL) {
        char* name = modcode + modip + 1;
        printf("CALL %s\n", name);
    } else if (modcode[modip] == OPCODE_END_CALL_ARGS) {
        printf("END_ARGS\n");
    } else if (modcode[modip] == OPCODE_CALL_ARG) {
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
    }
    modip++;
}

void menus_vm_call(char* name) {
    
}