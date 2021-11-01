#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#include "compiler.h"
#include "../args.h"
#include "ops.h"

struct parse_token_s1 {
    int   type;
    char* str;
    int   num;
};

struct module* compile(char* src) {
//    printf("reading source\n");
//    printf("%s\n", src);
//    printf("starting menus stage 1 parser\n");

    struct parse_token_s1* p1tokens = malloc(sizeof(struct parse_token_s1));
    int                    p1count  = 0;

    char* str;
    char* name;
    int   slen  = 0;
    int   nlen = 0;
    int   num;
    int   smode = 0;
    int   nmode = 0;
    int   comment = 0;

    for (int i = 0; i < strlen(src); i++) {
        if (src[i] == '/' && src[i+1] == '/') {
            comment = 1;
        } else if (comment == 1) {
            if (src[i] == '\n') {
                comment = 0;
//                printf("\n");
            }
        } else if (src[i] == '"') {
            if (smode == 0) {
                smode = 1;
                slen = 0;
                str = malloc(1);
            } else {
                smode = 0;
                str[slen] = 0;
                p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
                p1tokens[p1count].str = malloc(strlen(str) + 1);
                strcpy(p1tokens[p1count].str, str);
                p1tokens[p1count].type = 1;
                p1count++;
//                printf("[STRING \"%s\"]", str);
            }
        } else if (smode == 1) {
            str = realloc(str, slen + 1);
            str[slen] = src[i];
            slen++;
        } else if (src[i] >= '0' && src[i] <= '9') {
            num = 0;
            while (src[i] >= '0' && src[i] <= '9') {
                num += src[i] - '0';
                num *= 10;
                i++;
            }
            i--;
            num /= 10;
//            printf("[INT32 %i]", num);
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].num = num;
            p1tokens[p1count].type = 2;
            p1count++;
        } else if (src[i] == '=') {
            if (src[i+1] == '=') {
//                printf("[EQUALS]");
                p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
                p1tokens[p1count].type = 3;
                p1count++;
                i++;
            } else {
//                printf("[ASSIGN]");
                p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
                p1tokens[p1count].type = 4;
                p1count++;
            }
        } else if (src[i] == '{') {
//            printf("[OPENING_BRACE]");
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].type = 5;
            p1count++;
        } else if (src[i] == '}') {
//            printf("[CLOSING_BRACE]");
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].type = 6;
            p1count++;
        } else if (src[i] == ';') {
//            printf("[SEMICOLON]");
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].type = 7;
            p1count++;
        } else if (src[i] == '(') {
//            printf("[OPENING_PAREN]");
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].type = 8;
            p1count++;
        } else if (src[i] == ')') {
//            printf("[CLOSING_PAREN]");
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].type = 9;
            p1count++;
        } else if (src[i] == 'v' && src[i+1] == 'o' && src[i+2] == 'i' && src[i+3] == 'd') {
//            printf("[VOID]");
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].type = 10;
            p1count++;
            i += 3;
        } else if (src[i] == 'i' && src[i+1] == 'n' && src[i+2] == 't') {
//            printf("[INTERRUPT]");
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].type = 11;
            p1count++;
            i += 2;
        } else if (src[i] == 'r' && src[i+1] == 'e' && src[i+2] == 't' && src[i+3] == 'u' && src[i+4] == 'r' && src[i+5] == 'n') {
//            printf("[RETURN]");
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].type = 12;
            p1count++;
            i += 5;
        } else if (src[i] == ' ') {
//            printf(" ");
        } else if (src[i] == '\n') {
//            printf("\n");
        } else if ((src[i] >= 'a' && src[i] <= 'z') || (src[i] >= 'A' && src[i] <= 'Z')) {
            name = malloc(1);
            while ((src[i] >= 'a' && src[i] <= 'z') || (src[i] >= 'A' && src[i] <= 'Z')) {
                name = realloc(name, nlen + 1);
                name[nlen] = src[i];
                i++;
                nlen++;
            }
            name[nlen] = 0;
            i--;
//            printf("[IDENT \"%s\"]", name);
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].str = malloc(strlen(name) + 1);
            strcpy(p1tokens[p1count].str, name);
            p1tokens[p1count].type = 13;
            p1count++;
            nlen = 0;
            free(name);
        }
    }

    uint32_t codelen = 0;
    uint32_t codepos = 0;
    char*    code    = malloc(4096);
    int      var_c = 0; // for shitty mangling
    int      cstackp = 0;
    char**   cstackv = malloc(sizeof(void*) * 16);
    int*     cstackt = malloc(sizeof(int) * 16);
    char***  cstackvars = malloc(sizeof(void*));
             cstackvars[0] = malloc(1);
    int*     cstackvarc = malloc(sizeof(int) * 16);
             cstackvarc[0] = 0;
#define C    code
#define P    codepos
#define L    codelen
#define T    p1tokens
#define M    var_c
#define CSP  cstackp
#define CST  cstackt
#define CSV  cstackv
#define CSVV cstackvars
#define CSVC cstackvarc

#define CS_INITIALIZER 0
#define CS_ARGS        1
#define CS_FUNCTION    2

#define TIS(a, b) T[a].type == b

    script_header_t* header  = (void*)code;
    codelen                 += sizeof(script_header_t);
    codepos                  = codelen;

    header->version = 1;
    
    for (int i = 0; i < p1count; i++) {
        if (TIS(i,13) && TIS(i+1,13) && TIS(i+2,5)) {
            char* ass = malloc(strlen(T[i+1].str) + 10);
            sprintf(ass, "%08x_%s", M, T[i+1].str);
//            printf("[DECLARE \"%s\" AS \"%s\"]\n", ass, T[i].str);
            CSVV[CSP] = realloc(CSVV[CSP], (CSVC[CSP]+1) * sizeof(void*));
            CSVV[CSP][CSVC[CSP]] = ass;
            CSVC[CSP]++;
            CSP++;
            CSVV[CSP] = malloc(1);
            CSVC[CSP] = 0;
            CSV[CSP] = ass;
            CST[CSP] = CS_INITIALIZER;
            WR_OP_VAR(C, P, L, ass, T[i].str);
            WR_OP_INITIALIZE(C, P, L, ass);
            M++;
            i += 2;
        } else if (TIS(i,6)) {
//            printf("}]\n");
            if (CST[CSP] == CS_INITIALIZER) {
                WR_OP_END_INITIALIZE(C, P, L);
            } else if (CST[CSP] == CS_FUNCTION) {
                WR_OP_END_FUNCTION(C, P, L);
            }
            CSP--;
        } else if (TIS(i,10) && TIS(i+1,13) && TIS(i+2,8) && TIS(i+3,9)) {
//            printf("[FUNCTION \"%s\" {\n", T[i+1].str);
            char* ass = malloc(strlen(T[i+1].str) + 10);
            sprintf(ass, "%08x_%s", M, T[i+1].str);
            CSVV[CSP] = realloc(CSVV[CSP], (CSVC[CSP]+1) * sizeof(void*));
            CSVV[CSP][CSVC[CSP]] = ass;
            CSVC[CSP]++;
            CSP++;
            CSVV[CSP] = malloc(1);
            CSVC[CSP] = 0;
            CSV[CSP] = ass;
            CST[CSP] = CS_FUNCTION;
            M++;
            WR_OP_FUNCTION(C, P, L, ass, "Void");
            if (CST[CSP-1] == CS_INITIALIZER) {
                WR_OP_SET_REF(C, P, L, T[i+1].str, ass);
            }
            i += 3;
        } else if (TIS(i, 11)) {
            WR_OP_INTERRUPT(C, P, L);
//            printf("[INTERRUPT]\n");
        } else if (TIS(i, 12)) {
            WR_OP_RETURN(C, P, L);
//            printf("[RETURN]\n");
        } else if (TIS(i, 13) && TIS(i+1, 4) && TIS(i+2, 1)) {
            char* ass = T[i].str;
            for (int ii = 0; ii < CSVC[CSP]; ii++) {
                if (strcmp(CSVV[CSP][ii] + 9, T[i].str) == 0) {
                    ass = CSVV[CSP][ii];
                    break;
                }
            }
//            printf("[SET \"%s\" TO STRING \"%s\"]\n", ass, T[i+2].str);
            WR_OP_SET_STRING(C, P, L, ass, T[i+2].str);
            i += 2;
        } else if (TIS(i, 13) && TIS(i+1, 4) && TIS(i+2, 2)) {
            char* ass = T[i].str;
            for (int ii = 0; ii < CSVC[CSP]; ii++) {
                if (strcmp(CSVV[CSP][ii] + 9, T[i].str) == 0) {
                    ass = CSVV[CSP][ii];
                    break;
                }
            }
//            printf("[SET \"%s\" TO INT32 %i]\n", T[i].str, T[i+2].num);
            WR_OP_SET_INT32(C, P, L, ass, T[i+2].num);
            i += 2;
        } else if (TIS(i, 13) && TIS(i+1, 8)) {
//            printf("[CALL \"%s\" WITH ARGS (", T[i].str);
            CSP++;
            CST[CSP] = CS_ARGS;
            WR_OP_CALL(C, P, L, T[i].str);
            i++;
        } else if (TIS(i, 9)) {
            CSP--;
//            printf(")]\n");
            WR_OP_END_CALL_ARGS(C, P, L);
        } else if (TIS(i, 13)) {
            if (CST[CSP] == CS_ARGS) {
                char* ass = T[i].str;
                for (int iii = CSP - 1; iii >= 0; iii--) {
//                    printf("1 0x%08x\n", iii);
                    for (int ii = 0; ii < CSVC[iii]; ii++) {
//                        printf("2 0x%08x %s\n", ii, CSVV[iii][ii]);
                        if (strcmp(CSVV[iii][ii] + 9, T[i].str) == 0) {
                            ass = CSVV[iii][ii];
                            break;
                        }
                    }
                }
                WR_OP_CALL_ARG_REF(C, P, L, ass);
            }
//            printf("[REF \"%s\"]", T[i].str);
        } else {

        }
//        printf("L = 0x%08x\nP = 0x%08x\n", L, P);
//        printf("compiler dump\n");
//        for (int i = 0; i < L; i++) {
//            printf("%c", C[i]);
//        }
//        printf("\n");
    }

    if (arg_dump_code) {
        remove(arg_dump_code_file);
        printf("dumping bytecode to %s\n", arg_dump_code_file);
        int dump = open(arg_dump_code_file, O_WRONLY | O_CREAT);
        write(dump, code, codelen);
        close(dump);
        chmod(arg_dump_code_file, S_IROTH | S_IWOTH | S_IWGRP | S_IRGRP | S_IWUSR | S_IRUSR);
    }

    struct module* hi = malloc(sizeof(struct module));
    hi->code = code;
    hi->lengrh = codelen;
    return hi;
}