#ifndef STATIC_ASSERT_H
#define STATIC_ASSERT_H

#ifndef static_assert
#  if __STDC_VERSION__ >= 201112L
#    define static_assert _Static_assert
#  else
#    define SA_PPCAT_F(x, y) x ## y
#    define SA_PPCAT(x, y) SA_PPCAT_F(x, y)
#    define static_assert(expression, message) \
typedef char SA_PPCAT(static_assert_array_, __LINE__) \
[(expression) && (message) ? 1 : -1]
#  endif
#endif

#endif /* STATIC_ASSERT_H */