/* Defer statement for ANSI C
 *
 * Macros:
 *   DEFER_STACK_SIZE   Maximum count of defer statements (overridable)
 *   defer_init         Definition variable for implementation
 *   defer(...)         Definition defer statement
 *   defer_return       Replacement for casual return
 *
 * Example:
 *   #include <stdio.h>
 *   #include "defer.h"
 *
 *   int main(void) {
 *       defer_init;
 *
 *       puts("Init system");
 *       defer(puts("Clean system"););
 *
 *       puts("Open file");
 *       defer(puts("Close file"););
 *
 *       puts("Read by format");
 *       defer(puts("Free temporaly buffer"););
 *
 *       puts("Do some stuff");
 *
 *       defer_return 0;
 *   }
 *
 * Output:
 *   Init system
 *   Open file
 *   Read by format
 *   Do some stuff
 *   Free temporaly buffer
 *   Close file
 *   Clean system
 */

#ifndef DEFER_H
#define DEFER_H

#include <setjmp.h>

#ifndef DEFER_STACK_SIZE
#define DEFER_STACK_SIZE 15
#endif

#define defer_init \
    jmp_buf __defer_return, __defer_stack[DEFER_STACK_SIZE]; int __defer_sp = 0;

#define defer(...) do { \
    if (setjmp(__defer_stack[__defer_sp])) { __VA_ARGS__ }    \
    else { ++__defer_sp; break; }                             \
                                                              \
    if (__defer_sp == 0) longjmp(__defer_return, 1);          \
    else --__defer_sp, longjmp(__defer_stack[__defer_sp], 1); \
} while (0)

#define defer_return \
    if (setjmp(__defer_return) == 0) {                           \
        if (__defer_sp > 0)                                      \
            --__defer_sp, longjmp(__defer_stack[__defer_sp], 1); \
    } else return /* Here your return expression */

#endif /* DEFER_H */