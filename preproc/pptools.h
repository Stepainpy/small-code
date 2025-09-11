#ifndef PREPROCESSOR_TOOLS_H
#define PREPROCESSOR_TOOLS_H

#define _USE_MATH_DEFINES
#include <limits.h>
#include <math.h>

#define PPT_PPCAT_F(x, y) x ## y
#define PPT_PPCAT(x, y) PPT_PPCAT_F(x, y)

#define PPT_PPSTR_F(x) # x
#define PPT_PPSTR(x) PPT_PPSTR_F(x)

#define ppt_countof(array) (sizeof(array) / sizeof((array)[0]))
#define ppt_bitsof(value) (sizeof(value) * CHAR_BIT)

#define ppt_mod(x, y) (((x) % (y) + (y)) % (y))

#define ppt_deg_to_rad(deg) ((deg) / 180. * M_PI)
#define ppt_rad_to_deg(rad) ((rad) / M_PI * 180.)

#ifdef PPTOOLS_STRIP_PREFIX
#  define PPCAT PPT_PPCAT
#  define PPSTR PPT_PPSTR
#  define countof ppt_countof
#  define bitsof ppt_bitsof
#  define mod ppt_mod
#  define deg_to_rad ppt_deg_to_rad
#  define rad_to_deg ppt_rad_to_deg
#endif /* PPTOOLS_STRIP_PREFIX */

#endif /* PREPROCESSOR_TOOLS_H */