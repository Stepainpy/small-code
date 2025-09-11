#ifndef ASSERT_FORMATTED_H
#define ASSERT_FORMATTED_H

#include <stdio.h>
#include <stdlib.h>

#ifndef NDEBUG
#define assertf(expression, format, ...)      \
(void)(!!(expression) || (fprintf(stderr,     \
"%s:%i: Assertion failed: " #expression " | " \
"Message: " format "\n", __FILE__, __LINE__,  \
## __VA_ARGS__), exit(EXIT_FAILURE), 0))
#else
#define assertf(expression, format, ...) ((void)0)
#endif /* NDEBUG */

#endif /* ASSERT_FORMATTED_H */