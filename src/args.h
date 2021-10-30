#ifndef MENUS_ARGS_H
#define MENUS_ARGS_H

extern int   arg_dump_code;
extern int   arg_has_src;
extern char* arg_src;
extern char* arg_dump_code_file;

void parse_args(int argc, char** argv);

#endif