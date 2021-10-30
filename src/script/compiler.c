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

char* compile(char* src) {
    printf("reading source\n");
    printf("%s\n", src);
    printf("starting menus stage 1 parser\n");

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
                printf("\n");
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
                printf("[STRING \"%s\"]", str);
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
            printf("[NUMBER %i]", num);
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].num = num;
            p1tokens[p1count].type = 2;
            p1count++;
        } else if (src[i] == '=') {
            if (src[i+1] == '=') {
                printf("[EQUALS]");
                p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
                p1tokens[p1count].type = 3;
                p1count++;
                i++;
            } else {
                printf("[ASSIGN]");
                p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
                p1tokens[p1count].type = 4;
                p1count++;
            }
        } else if (src[i] == '{') {
            printf("[OPENING_BRACE]");
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].type = 5;
            p1count++;
        } else if (src[i] == '}') {
            printf("[CLOSING_BRACE]");
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].type = 6;
            p1count++;
        } else if (src[i] == ';') {
            printf("[SEMICOLON]");
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].type = 7;
            p1count++;
        } else if (src[i] == '(') {
            printf("[OPENING_PAREN]");
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].type = 8;
            p1count++;
        } else if (src[i] == ')') {
            printf("[CLOSING_PAREN]");
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].type = 9;
            p1count++;
        } else if (src[i] == 'v' && src[i+1] == 'o' && src[i+2] == 'i' && src[i+3] == 'd') {
            printf("[VOID]");
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].type = 10;
            p1count++;
            i += 3;
        } else if (src[i] == 'i' && src[i+1] == 'n' && src[i+2] == 't') {
            printf("[INTERRUPT]");
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].type = 11;
            p1count++;
            i += 2;
        } else if (src[i] == 'r' && src[i+1] == 'e' && src[i+2] == 't' && src[i+3] == 'u' && src[i+4] == 'r' && src[i+5] == 'n') {
            printf("[RETURN]");
            p1tokens = realloc(p1tokens, sizeof(struct parse_token_s1) * (p1count + 1));
            p1tokens[p1count].type = 12;
            p1count++;
            i += 5;
        } else if (src[i] == ' ') {
            printf(" ");
        } else if (src[i] == '\n') {
            printf("\n");
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
            printf("[IDENT \"%s\"]", name);
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

    script_header_t* header  = (void*)code;
    codelen                 += sizeof(script_header_t);
    codepos                  = codelen;

    header->version = 1;
    
    printf("starting menus stage 2 parser\n");

    for (int i = 0; i < p1count; i++) {
        if (p1tokens[i].type == 13 && p1tokens[i+1].type == 13 && p1tokens[i+2].type == 5) {
            char* ass = malloc(strlen(T[i+1].str) + 10);
            sprintf(ass, "%08x_%s", M, T[i+1].str);
            printf("[DECLARE \"%s\" AS \"%s\"]\n", ass, T[i].str);
            CSVV[CSP] = realloc(CSVV[CSP], (CSVC[CSP]+1) * sizeof(void*));
            CSVV[CSP][CSVC[CSP]] = ass;
            CSVC[CSP]++;
            CSP++;
            CSVV[CSP] = malloc(1);
            CSVC[CSP] = 0;
            CSV[CSP] = ass;
            CST[CSP] = CS_INITIALIZER;
            WR_OP_VAR(C, P, L, ass, T[i].str);
            M++;
            i += 2;
        } else if (p1tokens[i].type == 6) {
            printf("}]\n");
            cstackp--;
        } else if (p1tokens[i].type == 10 && p1tokens[i+1].type == 13 && p1tokens[i+2].type == 8 && p1tokens[i+3].type == 9) {
            printf("[FUNCTION \"%s\" {\n", p1tokens[i+1].str);
            CSP++;
            i += 3;
        } else if (p1tokens[i].type == 11) {
            printf("[INTERRUPT]\n");
        } else if (p1tokens[i].type == 12) {
            printf("[RETURN]\n");
        } else if (p1tokens[i].type == 13 && p1tokens[i+1].type == 4 && p1tokens[i+2].type == 1) {
            char* ass = T[i].str;
            for (int ii = 0; ii < CSVC[CSP]; ii++) {
                if (strcmp(CSVV[CSP][ii] + 9, T[i].str) == 0) {
                    ass = CSVV[CSP][ii];
                    break;
                }
            }
            printf("[SET \"%s\" TO STRING \"%s\"]\n", ass, T[i+2].str);
            i += 2;
        } else if (p1tokens[i].type == 13 && p1tokens[i+1].type == 4 && p1tokens[i+2].type == 2) {
            printf("[SET \"%s\" TO NUMBER %i]\n", p1tokens[i].str, p1tokens[i+2].num);
            i += 2;
        } else if (p1tokens[i].type == 13 && p1tokens[i+1].type == 8) {
            printf("[CALL \"%s\" WITH ARGS (", p1tokens[i].str);
            i++;
        } else if (p1tokens[i].type == 9) {
            printf(")]\n");
        } else if (p1tokens[i].type == 13) {
            printf("[REF \"%s\"]", p1tokens[i].str);
        }
    }

    if (arg_dump_code) {
        remove(arg_dump_code_file);
        printf("dumping bytecode to %s\n", arg_dump_code_file);
        int dump = open(arg_dump_code_file, O_WRONLY | O_CREAT);
        write(dump, code, codelen);
        close(dump);
        chmod(arg_dump_code_file, S_IROTH | S_IWOTH | S_IWGRP | S_IRGRP | S_IWUSR | S_IRUSR);
    }

    return code;
}