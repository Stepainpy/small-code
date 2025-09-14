/* Trilean details:
 * 1. Use balanced ternary
 *   false   (F) = -1
 *   unknown (U) =  0
 *   true    (T) = +1
 * 2. Gates NOT/AND/OR is NEG/MIN/MAX
 * 3. Truth tables
 *   NOT(A) = FUT -> TUF
 *
 *     AND(A, B)     OR(A, B)      XOR(A, B)
 *   A/B F  U  T   A/B F  U  T   A/B F  U  T
 *    F  F  F  F    F  F  U  T    F  F  U  T
 *    U  F  U  U    U  U  U  T    U  U  U  U
 *    T  F  U  T    T  T  T  T    T  T  U  F
 */

#ifndef TRILEAN_H
#define TRILEAN_H

typedef signed char trilean_t;

#define TLFALSE   ((trilean_t)-1)
#define TLUNKNOWN ((trilean_t) 0)
#define TLTRUE    ((trilean_t)+1)

#define TLNOT(a   ) (-(a))
#define TLAND(a, b) ((a) < (b) ? (a) : (b))
#define TLOR( a, b) ((a) > (b) ? (a) : (b))
#define TLXOR(a, b) TLAND(TLOR(a, b), TLNOT(TLAND(a, b)))

#define TRILEAN2BOOL(a) ((a) > TLUNKNOWN)

#endif /* TRILEAN_H */