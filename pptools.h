#ifndef PREPROCESSOR_TOOLS_H
#define PREPROCESSOR_TOOLS_H

#define PPT_PPCAT_F(x, y) x ## y
#define PPT_PPCAT(x, y) PPT_PPCAT_F(x, y)

#define PPT_PPSTR_F(x) # x
#define PPT_PPSTR(x) PPT_PPSTR_F(x)

#define ppt_countof(array) (sizeof(array) / sizeof((array)[0]))

#define ppt_mod(x, y) (((x) % (y) + (y)) % (y))

#ifdef PPTOOLS_STRIP_PREFIX
#  define PPCAT PPT_PPCAT
#  define PPSTR PPT_PPSTR
#  define countof ppt_countof
#  define mod ppt_mod
#endif /* PPTOOLS_STRIP_PREFIX */

#endif /* PREPROCESSOR_TOOLS_H */