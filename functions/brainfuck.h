#ifndef BRAINFUCK_INTERPRETER_H
#define BRAINFUCK_INTERPRETER_H

int brainfuck(const char* code);

#endif /* BRAINFUCK_INTERPRETER_H */

#ifdef BRAINFUCK_IMPLEMENTATION

#include <stdio.h>
#include <string.h>

int brainfuck(const char* code) {
    static unsigned char mem[0x8000];
    unsigned char* end = mem + sizeof mem;
    unsigned char* cur = mem; int ch;
    const char * start = code;
    memset(mem, 0, sizeof mem);

    if (!code) return 1;
    while (*code && mem <= cur && cur < end) {
        switch (*code++) {
            case '+': ++(*cur); break;
            case '-': --(*cur); break;
            case '>': ++  cur ; break;
            case '<': --  cur ; break;

            case '.': fputc(*cur, stdout); break;
            case ',': *cur = (ch = fgetc(stdin)) != EOF ? ch : 0; break;

            case '[': {
                int depth = 1;
                if (*cur != 0) continue;
                while (*code && depth > 0) {
                    if (*code == ']') --depth;
                    if (*code == '[') ++depth;
                    ++code;
                }
                if (depth != 0) return 1;
            } break;

            case ']': {
                int depth = 1;
                if (*cur == 0) continue;
                code -= 2;
                while (start <= code && depth > 0) {
                    if (*code == ']') ++depth;
                    if (*code == '[') --depth;
                    --code;
                }
                if (depth != 0) return 1;
                code += 2;
            } break;
        }
    }

    return *code != '\0';
}

#endif /* BRAINFUCK_IMPLEMENTATION */