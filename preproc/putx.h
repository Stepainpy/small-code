#ifndef PUTX_H
#define PUTX_H

#include <stdio.h>

#define __putx_fmt(x) _Generic((x), \
              char : "%c"  , \
       signed char : "%hhi", \
     unsigned char : "%hhu", \
      signed short : "%hi" , \
    unsigned short : "%hu" , \
        signed int : "%i"  , \
      unsigned int : "%u"  , \
       signed long : "%li" , \
     unsigned long : "%lu" , \
  signed long long : "%lli", \
unsigned long long : "%llu", \
             float : "%f"  , \
            double : "%lf" , \
       long double : "%Lf" , \
             void* : "%p"  , \
             char* : "%s"  , \
       const char* : "%s"  , \
           default : "%%"  )

#define putx(value) printf(__putx_fmt(value), (value))
#define snputx(buf, size, value) \
snprintf((buf), (size), __putx_fmt(value), (value))
#define fputx(stream, value) \
fprintf((stream), __putx_fmt(value), (value))

#define putxn(value) (putx(value), printf("\n"))

#endif // PUTX_H