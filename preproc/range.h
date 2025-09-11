#ifndef RANGE_H
#define RANGE_H

/* suffixs:
 * rft
 * ||+- with type (default size_t)
 * |+-- full range (default with begin/end 0)
 * +--- reverse direction
 *
 * range contains:
 * fwd = [begin; end)
 * rev = (begin; end]
 *
 * usage:
 * for range(i, 10) { ... }
 */

#define rangeft(type, ivar, begin, end) \
(type ivar = (begin); ivar < (end); ++ivar)

#define rangerft(type, ivar, begin, end) \
(type ivar = (begin); ivar --> (end);)

#define rangef( ivar, begin, end) \
rangeft (size_t, ivar, begin, end)
#define rangerf(ivar, begin, end) \
rangerft(size_t, ivar, begin, end)

#define range( ivar, end)   rangef (ivar, 0, end)
#define ranger(ivar, begin) rangerf(ivar, begin, 0)

#define ever (;;)

#endif // RANGE_H