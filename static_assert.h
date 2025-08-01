#ifndef STATIC_ASSERT_H
#define STATIC_ASSERT_H

#define CONCAT_(x, y) x ## y
#define CONCAT(x, y) CONCAT_(x, y)

#ifndef static_assert
#define static_assert(cond, msg) \
typedef char CONCAT(static_assert_, __LINE__)[!!(cond) * 2 - 1]
#endif

#endif /* STATIC_ASSERT_H */