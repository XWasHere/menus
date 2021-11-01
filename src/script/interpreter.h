#ifndef MENUS_SCRIPT_INTERPRETER_H
#define MENUS_SCRIPT_INTERPRETER_H

#include "compiler.h"

void menus_vm_load(module_t*);
void menus_vm_exec_instr();
void menus_vm_call(char*);

#endif