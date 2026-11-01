#ifndef ASCII_HISTOGRAM_H
#define ASCII_HISTOGRAM_H

#include <stddef.h>

/* Drawing histogram for y in [0; 1]
 * `width`  - count of characters in one column
 * `height` - count of rows in one column
 * `scale`  - if equal 0 then assing 1/max from `columns`
 */
void histogram(const double columns[], size_t count, size_t width, size_t height, double scale);

#endif /* ASCII_HISTOGRAM_H */

#ifdef HISTOGRAM_IMPLEMENTATION

#include <stdio.h>

void histogram(const double columns[], size_t count, size_t width, size_t height, double scale) {
    static const char levels[9][4] = {
        "\x20\x00\x00",
        "\xe2\x96\x81",
        "\xe2\x96\x82",
        "\xe2\x96\x83",
        "\xe2\x96\x84",
        "\xe2\x96\x85",
        "\xe2\x96\x86",
        "\xe2\x96\x87",
        "\xe2\x96\x88",
    };

    const double gap = 1. / (double)height;
    size_t i, j, k;

    if (scale == +0.0 || scale == -0.0) {
        double max = -1. / 0.; /* -inf */
        for (i = 0; i < count; i++)
            max = columns[i] > max ? columns[i] : max;
        scale = 1. / max;
    }

    puts("1^");
    for (i = height; i --> 0;) {
        fputs(" |", stdout);
        for (j = 0; j < count; j++) {
            const double upper = scale * columns[j] - gap * (i + 1);
            const double lower = scale * columns[j] - gap * (i + 0);
            /**/ if (upper > 0) for (k = 0; k < width; k++) fputs(levels[8], stdout);
            else if (lower < 0) for (k = 0; k < width; k++) fputs(levels[0], stdout);
            else {
                double t = lower / gap;
                for (k = 0; k < width; k++)
                    fputs(levels[(size_t)(t * 8)], stdout);
            }
        }
        putchar('\n');
    }
    fputs("0+", stdout);
    for (i = 0; i < count * width; i++) fputc('-', stdout);
    puts(">");
}

#endif /* HISTOGRAM_IMPLEMENTATION */