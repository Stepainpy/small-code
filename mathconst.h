#ifndef MATH_CONSTANTS_H
#define MATH_CONSTANTS_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.71828182845904523536
#endif

#define DEG2RAD(deg) ((deg) / 180. * M_PI)
#define RAD2DEG(rad) ((rad) / M_PI * 180.)

#endif /* MATH_CONSTANTS_H */