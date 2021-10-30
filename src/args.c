#include <string.h>

int arg_dump_code;
int arg_has_src;

char* arg_src;
char* arg_dump_code_file;

void parse_args(int argc, char** argv) {
    arg_dump_code = 0;
    arg_has_src = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--dump-code-to") == 0) {
            arg_dump_code = 1;
            arg_dump_code_file = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "--dump-code") == 0) {
            arg_dump_code = 1;
            arg_dump_code_file = "./a.dmp";
        } else if (strcmp(argv[i], "--src") == 0) {
            arg_has_src = 1;
            arg_src = argv[i + 1];
            i++;
        }
    }
}